#include <iostream>
#include <unistd.h>
#include <deque>

#include "kafka_client.h"
#include "network.h"
#include "util.h"
#include "response.h"
#include "net_util.h"

Network::Network()
{
}

Network::~Network()
{
	for (auto it = fds_.begin(); it != fds_.end(); ++it)
		close(*it);

	// delete nodes
	for (auto it = nodes_.begin(); it != nodes_.end(); ++it)
		//delete it->second;

	pthread_mutex_destroy(&queue_mutex_);
}

int Network::Init(KafkaClient *client, const std::string &broker_list)
{
	pthread_mutex_init(&queue_mutex_, NULL);
	client_ = client;

	// create nodes
	int tmp_broker_id = -1;
	std::vector<std::string> brokers = Util::Split(broker_list, ',');
	for (auto b_it = brokers.begin(); b_it != brokers.end(); ++b_it)
	{
		std::vector<std::string> host_port = Util::Split(*b_it, ':');
		std::string host = host_port[0];
		std::string ip = Util::HostnameToIp(host);
		int port = std::stoi(host_port[1]);
		int fd = NetUtil::NewTcpClient(ip.c_str(), port);
		fds_.push_back(fd);
		Node *node = new Node(fd, -1, host, port);
		nodes_.insert({tmp_broker_id--, node});
	}

	//state_machine_ = new StateMachine(network_, nodes_);
	//state_machine_->Init();

	std::cout << "Network init OK!" << std::endl;

	return 0;
}

int Network::Start()
{
	//while(1)
	{
		Response *response;

		std::vector<std::string> topics({"test"});
		MetadataRequest *metadata_request = new MetadataRequest(2, topics);
		// Random select a node
		auto it = nodes_.begin();
		std::advance(it, rand() % nodes_.size());
		Node *node = it->second;
		SendRequestHandler(node, metadata_request);
		ReceiveResponseHandler(node, &response);
		MetadataResponse *meta_response = dynamic_cast<MetadataResponse*>(response);
		// insert new node
		for (auto n_it = nodes_.begin(); n_it != nodes_.end(); ++n_it)
		{
			Node *node = n_it->second;
			int broker_id = meta_response->GetBrokerIdFromHostname(node->host_);
			node->id_ = broker_id;
			nodes_.insert({broker_id, node});
		}
		// delete old node
		for (auto n_it = nodes_.begin(); n_it != nodes_.end(); /* NULL */)
		{
			if (n_it->first < 0)
				n_it = nodes_.erase(n_it);
			else
				++n_it;
		}
		std::vector<PartitionMetadata> &partition_metadata =
							meta_response->topic_metadata_[0].partition_metadata_;
		for (auto pm_it = partition_metadata.begin(); pm_it != partition_metadata.end(); ++pm_it)
		{
			Partition partition(pm_it->partition_id_, pm_it->leader_);
			partitions_map_.insert({partition.id_, partition});
		}
		

		GroupCoordinatorRequest *group_request = new GroupCoordinatorRequest(0, "group");
		SendRequestHandler(node, group_request);
		ReceiveResponseHandler(node, &response);
		GroupCoordinatorResponse *group_response = dynamic_cast<GroupCoordinatorResponse*>(response);
		node = nodes_[group_response->coordinator_id_];
		
		JoinGroupRequest *join_request = new JoinGroupRequest(0, "group", "", topics);
		SendRequestHandler(node, join_request);
		ReceiveResponseHandler(node, &response);
		JoinGroupResponse *join_response = dynamic_cast<JoinGroupResponse*>(response);
		generation_id_ = join_response->GetGenerationId();
		member_id_ = join_response->GetMemberId();
		members_ = join_response->GetAllMembers();
		PartitionAssignment();


		SyncGroupRequest *sync_request = new SyncGroupRequest(0, "test", "group", generation_id_,
				member_id_, member_partition_map_);
		sync_request->PrintAll();
		SendRequestHandler(node, sync_request);
		ReceiveResponseHandler(node, &response);

		HeartbeatRequest *hear_request = new HeartbeatRequest(1, "group", generation_id_, member_id_);
		hear_request->PrintAll();
		SendRequestHandler(node, hear_request);
		ReceiveResponseHandler(node, &response);

		std::vector<int> partitions({1});
		OffsetFetchRequest *offset_request = new OffsetFetchRequest(1, "group", "test", partitions);
		offset_request->PrintAll();
		SendRequestHandler(node, offset_request);
		ReceiveResponseHandler(node, &response);
		OffsetFetchResponse *offset_response = dynamic_cast<OffsetFetchResponse*>(response);
		offset_response->ParseOffset(partition_offset_map_);

		FetchRequest *fetch_request = new FetchRequest(0, "test", 1, partition_offset_map_[1]);
		fetch_request->PrintAll();
		SendRequestHandler(node, fetch_request);
		ReceiveResponseHandler(node, &response);

		delete response;
	}

	return 0;
}

int Network::Stop()
{
	return 0;
}

int Network::ReceiveResponseHandler(Node *node, Response **response)
{
	int ret = Receive(node->fd_, response);
	if (ret == 0)
	{
		(*response)->PrintAll();
		delete last_request_;
	}

	return 0;
}

int Network::SendRequestHandler(Node *node, Request *request)
{
	Send(node->fd_, request);

	last_request_ = request;

	return 0;
}

int Network::Receive(int fd, Response **res)
{
	char buf[6000] = {0};

	int ret = CompleteRead(fd ,buf);
	if (ret < 0)
		return -1;

	int response_size = Util::NetBytesToInt(buf);
	int correlation_id = Util::NetBytesToInt(buf + 4); 
	int api_key = GetApiKeyFromResponse(last_request_, correlation_id);
	if (api_key < 0)
	{
		// not match
		return -1;
	}

	char *p = buf;
	switch(api_key)
	{
		case ApiKey::MetadataRequest:
		{
			MetadataResponse *response = new MetadataResponse(&p);
			*res = response;
			break;
		}
		case ApiKey::GroupCoordinatorRequest:
		{

			GroupCoordinatorResponse *response = new GroupCoordinatorResponse(&p);
			*res = response;
			break;
		}
		case ApiKey::JoinGroupRequest:
		{
			JoinGroupResponse *response = new JoinGroupResponse(&p);
			*res = response;
			break;
		}
		case ApiKey::SyncGroupRequest:
		{
			SyncGroupResponse *response = new SyncGroupResponse(&p);
			*res = response;
			break;
		}
		case ApiKey::HeartbeatRequest:
		{
			HeartbeatResponse *heart_response = new HeartbeatResponse(&p);
			*res = heart_response;
			break;
		}
		case ApiKey::FetchRequest:
		{
			FetchResponse *fetch_response = new FetchResponse(&p);
			*res = fetch_response;
			break;
		}
		case ApiKey::OffsetFetchRequest:
		{
			OffsetFetchResponse *offset_fetch_response = new OffsetFetchResponse(&p);
			*res = offset_fetch_response;
			break;
		}
	}

	return 0;
}

int Network::Send(int fd, Request *request)
{
	int packet_size = request->CountSize() + 4;
	char buf[2048];
	char *p = buf;

#if 0
	switch(request->api_key_)
	{
		case ApiKey::MetadataRequest:
		{
			request->Package(&p);
			break;
		}
		case ApiKey::GroupCoordinatorRequest:
		{
			request->Package(&p);
			break;
		}
		case ApiKey::JoinGroupRequest:
		{
			request->Package(&p);
			break;
		}
		case ApiKey::SyncGroupRequest:
		{
			request->Package(&p);
			break;
		}
	}
#endif

	request->Package(&p);

	int nwrite = write(fd, buf, packet_size);
	std::cout << "Send " << nwrite << " bytes" << std::endl;
	if (nwrite < 0)
	{
		if (errno == EWOULDBLOCK)
		{
			return 0;
		}
		else
		{
			std::cerr << "An error has occured while writing to the server." << std::endl;
			return -1;
		}
	}

	return 0;
}

short Network::GetApiKeyFromResponse(Request *last_request, int correlation_id)
{
	if (last_request->correlation_id_ != correlation_id)
	{
		std::cout << "The correlation_id are not equal" << std::endl;
		return -1;
	}

	return last_request->api_key_;
}

int Network::PartitionAssignment()
{
	int base = partitions_map_.size() / members_.size();
	int remainder = partitions_map_.size() % members_.size();
	auto pm_it = partitions_map_.begin();
	
	for (auto m_it = members_.begin(); m_it != members_.end(); ++m_it)
	{
		std::vector<int> owned;
		for (int i = 0; i < base; i++)
		{
			owned.push_back(pm_it->second.id_);
			++pm_it;
		}

		if (remainder-- > 0)
		{
			owned.push_back(pm_it->second.id_);
			++pm_it;
		}

		member_partition_map_.insert({*m_it, owned});
	}

	return 0;
}

int Network::CompleteRead(int fd, char *buf)
{
	char size_buf[4];

	read(fd, size_buf, 4);
	memcpy(buf, size_buf, 4);
	int total_len = Util::NetBytesToInt(size_buf);
	std::cout << "total len = " << total_len << std::endl;

	int sum_read = 0;
	while (sum_read != total_len)
	{
		char tmp_buf[4096] = {0};
		int nread = read(fd, tmp_buf, 4096);
		std::cout << "nread = " << nread << std::endl;

		if (nread <= 0)
		{
			if (nread == 0)
				std::cout << "connection has been closed" << std::endl;
			else
				std::cout << "error occurred" << std::endl;

			close(fd);
			return -1;
		}

		memcpy(buf + 4 + sum_read, tmp_buf, nread);
		sum_read += nread;
	}

	return 0;
}


