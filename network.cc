#include <iostream>
#include <unistd.h>
#include <deque>

#include "kafka_client.h"
#include "network.h"
#include "util.h"
#include "net_util.h"
#include "response.h"
#include "offset_request.h"
#include "offset_response.h"
#include "offset_fetch_request.h"
#include "offset_fetch_response.h"
#include "fetch_request.h"
#include "fetch_response.h"
#include "metadata_request.h"
#include "metadata_response.h"
#include "heartbeat_request.h"
#include "heartbeat_response.h"
#include "sync_group_request.h"
#include "sync_group_response.h"
#include "join_group_request.h"
#include "join_group_response.h"
#include "group_coordinator_request.h"
#include "group_coordinator_response.h"

Network::Network(KafkaClient *client, const std::string &broker_list, const std::string &topic, const std::string &group)
{
	client_ = client;

	// create brokers
	int tmp_broker_id = -1;
	std::vector<std::string> brokers = Util::Split(broker_list, ',');
	for (auto b_it = brokers.begin(); b_it != brokers.end(); ++b_it)
	{
		std::vector<std::string> host_port = Util::Split(*b_it, ':');
		std::string host = host_port[0];
		std::string ip = Util::HostnameToIp(host);
		int port = std::stoi(host_port[1]);
		int fd = NetUtil::NewTcpClient(ip.c_str(), port);
		Broker broker(fd, -1, host, port);
		brokers_.insert({tmp_broker_id--, broker});
	}

	topic_ = topic;
	group_ = group;

	//pthread_mutex_init(&queue_mutex_, NULL);
	//state_machine_ = new StateMachine(network_, brokers_);
	//state_machine_->Init();

	std::cout << "Network init OK!" << std::endl;
}

Network::~Network()
{
	// close socket and delete brokers
	for (auto it = brokers_.begin(); it != brokers_.end(); ++it)
	{
		close(it->second.fd_);
		//delete it->second;
	}

	//pthread_mutex_destroy(&queue_mutex_);
}

int Network::Start()
{
	Response *response;

	std::vector<std::string> topics({topic_});
	MetadataRequest *metadata_request = new MetadataRequest(2, topics);
	// Random select a broker
	auto it = brokers_.begin();
	std::advance(it, rand() % brokers_.size());
	Broker *broker = &(it->second);
	SendRequestHandler(broker, metadata_request);
	ReceiveResponseHandler(broker, &response);
	MetadataResponse *meta_response = dynamic_cast<MetadataResponse*>(response);

	// insert new broker
	for (auto n_it = brokers_.begin(); n_it != brokers_.end(); ++n_it)
	{
		Broker b = n_it->second;
		int broker_id = meta_response->GetBrokerIdFromHostname(b.host_);
		b.id_ = broker_id;
		brokers_.insert({broker_id, b});
	}

	// delete old broker
	for (auto n_it = brokers_.begin(); n_it != brokers_.end(); /* NULL */)
	{
		if (n_it->first < 0)
			n_it = brokers_.erase(n_it);
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
	delete response;

	GroupCoordinatorRequest *group_request = new GroupCoordinatorRequest(0, group_);
	SendRequestHandler(broker, group_request);
	ReceiveResponseHandler(broker, &response);
	GroupCoordinatorResponse *group_response = dynamic_cast<GroupCoordinatorResponse*>(response);
	broker = &(brokers_[group_response->coordinator_id_]);
	delete response;
	

	JoinGroupRequest *join_request = new JoinGroupRequest(0, group_, "", topics);
	SendRequestHandler(broker, join_request);
	ReceiveResponseHandler(broker, &response);
	JoinGroupResponse *join_response = dynamic_cast<JoinGroupResponse*>(response);
	generation_id_ = join_response->GetGenerationId();
	member_id_ = join_response->GetMemberId();
	members_ = join_response->GetAllMembers();
	PartitionAssignment();
	delete response;


	SyncGroupRequest *sync_request = new SyncGroupRequest(0, topic_, group_, generation_id_,
			member_id_, member_partition_map_);
	sync_request->PrintAll();
	SendRequestHandler(broker, sync_request);
	ReceiveResponseHandler(broker, &response);
	delete response;

	HeartbeatRequest *hear_request = new HeartbeatRequest(1, group_, generation_id_, member_id_);
	hear_request->PrintAll();
	SendRequestHandler(broker, hear_request);
	ReceiveResponseHandler(broker, &response);
	delete response;

	std::vector<int> partitions({1});
	OffsetFetchRequest *offset_fetch_request = new OffsetFetchRequest(1, group_, topic_, partitions);
	offset_fetch_request->PrintAll();
	SendRequestHandler(broker, offset_fetch_request);
	ReceiveResponseHandler(broker, &response);
	OffsetFetchResponse *offset_fetch_response = dynamic_cast<OffsetFetchResponse*>(response);
	offset_fetch_response->ParseOffset(partition_offset_map_);
	delete response;

	for (auto po_it = partition_offset_map_.begin(); po_it != partition_offset_map_.end(); ++po_it)
	{
		if (po_it->second != -1)
			continue;

		std::vector<int> need_update_offset_partitions;
		need_update_offset_partitions.push_back(po_it->first);

		OffsetRequest *offset_request = new OffsetRequest(123, topic_, need_update_offset_partitions);
		offset_request->PrintAll();
		SendRequestHandler(broker, offset_request);
		ReceiveResponseHandler(broker, &response);
		OffsetResponse *offset_response = dynamic_cast<OffsetResponse*>(response);
		po_it->second = offset_response->GetNewOffset();
	}

	while (true)
	{
		FetchRequest *fetch_request = new FetchRequest(0, topic_, 1, partition_offset_map_[1]);
		fetch_request->PrintAll();
		SendRequestHandler(broker, fetch_request);
		ReceiveResponseHandler(broker, &response);
		delete response;
	}

	return 0;
}

int Network::Stop()
{
	return 0;
}

int Network::ReceiveResponseHandler(Broker *broker, Response **response)
{
	int ret = Receive(broker->fd_, response);
	if (ret == 0)
	{
		(*response)->PrintAll();
	}

	return 0;
}

int Network::SendRequestHandler(Broker *broker, Request *request)
{
	Send(broker->fd_, request);
	last_correlation_id_ = GetCorrelationIdFromRequest(request);
	last_api_key_ = GetApiKeyFromRequest(request);
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
	int api_key = GetApiKeyFromResponse(correlation_id);
	if (api_key < 0)
	{
		// not match
		return -1;
	}

	char *p = buf;
	switch(api_key)
	{
		case ApiKey::MetadataType:
		{
			MetadataResponse *response = new MetadataResponse(&p);
			*res = response;
			break;
		}
		case ApiKey::GroupCoordinatorType:
		{

			GroupCoordinatorResponse *response = new GroupCoordinatorResponse(&p);
			*res = response;
			break;
		}
		case ApiKey::JoinGroupType:
		{
			JoinGroupResponse *response = new JoinGroupResponse(&p);
			*res = response;
			break;
		}
		case ApiKey::SyncGroupType:
		{
			SyncGroupResponse *response = new SyncGroupResponse(&p);
			*res = response;
			break;
		}
		case ApiKey::HeartbeatType:
		{
			HeartbeatResponse *heart_response = new HeartbeatResponse(&p);
			*res = heart_response;
			break;
		}
		case ApiKey::FetchType:
		{
			FetchResponse *fetch_response = new FetchResponse(&p);
			*res = fetch_response;
			break;
		}
		case ApiKey::OffsetFetchType:
		{
			OffsetFetchResponse *offset_fetch_response = new OffsetFetchResponse(&p);
			*res = offset_fetch_response;
			break;
		}
		case ApiKey::OffsetType:
		{
			OffsetResponse *offset_response = new OffsetResponse(&p);
			*res = offset_response;
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

int Network::GetCorrelationIdFromRequest(Request *request)
{
	return request->GetCorrelationId();
}

short Network::GetApiKeyFromRequest(Request *request)
{
	return request->GetApiKey();
}

short Network::GetApiKeyFromResponse(int correlation_id)
{
	if (last_correlation_id_ != correlation_id)
	{
		std::cout << "The correlation_id are not equal" << std::endl;
		return -1;
	}

	return last_api_key_;
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


