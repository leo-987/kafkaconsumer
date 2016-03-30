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
#include "offset_commit_request.h"
#include "offset_commit_response.h"
#include "error_code.h"

#include "easylogging++.h"

_INITIALIZE_EASYLOGGINGPP

Network::Network(KafkaClient *client, const std::string &broker_list, const std::string &topic, const std::string &group)
{
	client_ = client;
	topic_ = topic;
	group_ = group;
	coordinator_ = NULL;
	amIGroupLeader_ = false;

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
		Broker broker(fd, tmp_broker_id, host, port);
		brokers_.insert({tmp_broker_id, broker});
		tmp_broker_id--;
	}

	//pthread_mutex_init(&queue_mutex_, NULL);

	event_ = Event::STARTUP;
	current_state_ = &Network::Initial;

	easyloggingpp::Configurations conf_from_file("easylogging.conf");
	easyloggingpp::Loggers::reconfigureAllLoggers(conf_from_file);
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
	while (1)
	{
		(this->*current_state_)(event_);
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
	request->PrintAll();
	Send(broker->fd_, request);
	last_correlation_id_ = request->GetCorrelationId();
	last_api_key_ = request->GetApiKey();
	return 0;
}

int Network::Receive(int fd, Response **res)
{
	//char buf[1048999] = {0};
	char size_buf[4];
	read(fd, size_buf, 4);
	int total_len = Util::NetBytesToInt(size_buf);
	//std::cout << "total len = " << total_len << std::endl;

	char *buf = new char[total_len + 4];
	memcpy(buf, size_buf, 4);

	int ret = CompleteRead(fd ,buf + 4, total_len);
	if (ret < 0)
		return -1;

	//int response_size = Util::NetBytesToInt(buf);
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
		case ApiKey::OffsetCommitType:
		{
			OffsetCommitResponse *commit_response = new OffsetCommitResponse(&p);
			*res = commit_response;
			break;
		}
	}

	delete[] buf;
	return 0;
}

int Network::Send(int fd, Request *request)
{
	int packet_size = request->CountSize() + 4;
	char buf[2048];
	char *p = buf;

	request->Package(&p);

	int nwrite = write(fd, buf, packet_size);
	//std::cout << "Send " << nwrite << " bytes" << std::endl;
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

short Network::GetApiKeyFromResponse(int correlation_id)
{
	if (last_correlation_id_ != correlation_id)
	{
		std::cout << "The correlation_id are not equal" << std::endl;
		return -1;
	}

	return last_api_key_;
}

/* Range:
 * p1 p2 p3 p4 p5
 *  \ /   \ /  |
 *   c1    c2  c3
 */
int Network::PartitionAssignment()
{
	int base = all_partitions_.size() / members_.size();
	int remainder = all_partitions_.size() % members_.size();
	auto pm_it = all_partitions_.begin();

	for (auto m_it = members_.begin(); m_it != members_.end(); ++m_it)
	{
		std::vector<int> owned;
		for (int i = 0; i < base; i++)
		{
			owned.push_back(pm_it->second.GetPartitionId());
			++pm_it;
		}
		if (remainder-- > 0)
		{
			owned.push_back(pm_it->second.GetPartitionId());
			++pm_it;
		}
		member_partition_map_.insert({*m_it, owned});
	}
	return 0;
}

int Network::CompleteRead(int fd, char *buf, int total_len)
{
	//char size_buf[4];
	//read(fd, size_buf, 4);
	//memcpy(buf, size_buf, 4);
	//int total_len = Util::NetBytesToInt(size_buf);
	//std::cout << "total len = " << total_len << std::endl;

	int sum_read = 0;
	while (sum_read != total_len)
	{
		char tmp_buf[1048576] = {0};
		int nread = read(fd, tmp_buf, sizeof(tmp_buf));
		//std::cout << "nread = " << nread << std::endl;

		if (nread <= 0)
		{
			if (nread == 0)
				std::cout << "connection has been closed" << std::endl;
			else
				std::cout << "error occurred" << std::endl;

			close(fd);
			return -1;
		}

		memcpy(buf + 0 + sum_read, tmp_buf, nread);
		sum_read += nread;
		//std::cout << sum_read << std::endl;
	}

	return 0;
}


//---------------------------state functions
int Network::Initial(Event &event)
{
	if (event != Event::STARTUP)
	{
		std::cout << __LINE__ << std::endl;
		return -1;
	}

	std::vector<std::string> topics({topic_});
	MetadataRequest *metadata_request = new MetadataRequest(topics);

	// Select the first broker
	auto b_it = brokers_.begin();
	Broker *broker = &(b_it->second);
	SendRequestHandler(broker, metadata_request);
	Response *response;
	ReceiveResponseHandler(broker, &response);
	MetadataResponse *meta_response = dynamic_cast<MetadataResponse*>(response);

	std::unordered_map<int, Broker> updated_brokers = meta_response->ParseBrokers(brokers_);
	if (!updated_brokers.empty())
	{
		brokers_ = updated_brokers;
		meta_response->ParsePartitions(all_partitions_);

		// next state
		current_state_ = &Network::DiscoverCoordinator;
		event = Event::DISCOVER_COORDINATOR;
	}
	else
	{
		// Sleep and retry metadata request in next loop
		sleep(5);
	}

	delete metadata_request;
	delete response;

	return 0;
}

int Network::DiscoverCoordinator(Event &event)
{
	// Select the first broker
	auto b_it = brokers_.begin();
	Broker *broker = &(b_it->second);

	Response *response;
	GroupCoordinatorRequest *group_request = new GroupCoordinatorRequest(group_);
	SendRequestHandler(broker, group_request);
	ReceiveResponseHandler(broker, &response);
	GroupCoordinatorResponse *group_response = dynamic_cast<GroupCoordinatorResponse*>(response);

	int32_t co_id = group_response->GetCoordinatorId();
	coordinator_ = &brokers_.at(co_id);

	delete group_request;
	delete response;

	// next state
	current_state_ = &Network::JoinGroup;
	event = Event::JOIN_WITH_EMPTY_CONSUMER_ID;

	return 0;
}

int Network::JoinGroup(Event &event)
{
	switch(event)
	{
		case Event::JOIN_WITH_EMPTY_CONSUMER_ID:
		{
			// first join
			std::vector<std::string> topics({topic_});
			std::string member_id = "";
			JoinGroupRequest *join_request = new JoinGroupRequest(group_, member_id, topics);
			SendRequestHandler(coordinator_, join_request);
			Response *response;
			ReceiveResponseHandler(coordinator_, &response);
			JoinGroupResponse *join_response = dynamic_cast<JoinGroupResponse*>(response);
			generation_id_ = join_response->GetGenerationId();
			member_id_ = join_response->GetMemberId();
			amIGroupLeader_ = join_response->IsGroupLeader();
			if (amIGroupLeader_ == true)
			{
				members_ = join_response->GetAllMembers();
				PartitionAssignment();
			}

			delete join_request;
			delete response;
			break;
		}
		case Event::JOIN_WITH_PREVIOUS_CONSUMER_ID:
		{
			// rejoin
			std::vector<std::string> topics({topic_});
			JoinGroupRequest *join_request = new JoinGroupRequest(group_, member_id_, topics);
			SendRequestHandler(coordinator_, join_request);
			Response *response;
			ReceiveResponseHandler(coordinator_, &response);
			JoinGroupResponse *join_response = dynamic_cast<JoinGroupResponse*>(response);
			generation_id_ = join_response->GetGenerationId();
			member_id_ = join_response->GetMemberId();
			amIGroupLeader_ = join_response->IsGroupLeader();
			if (amIGroupLeader_ == true)
			{
				members_ = join_response->GetAllMembers();
				PartitionAssignment();
			}

			delete join_request;
			delete response;
			break;
		}
		default:
		{
			break;
		}
	}

	// next state
	current_state_ = &Network::SyncGroup;
	event = Event::SYNC_GROUP;

	return 0;
}

// Create mapping: leader id -> [partitions]
std::map<int, std::vector<int>> Network::CreateBrokerIdToOwnedPartitionMap(const std::vector<int> &owned_partitions)
{
	std::map<int, std::vector<int>> result;
	for (auto p_it = owned_partitions.begin(); p_it != owned_partitions.end(); ++p_it)
	{
		int partition_id = *p_it;
		int leader_id = all_partitions_.at(partition_id).GetLeaderId();
		result[leader_id].push_back(partition_id);
	}
	return result;
}

int Network::SyncGroup(Event &event)
{
	SyncGroupRequest *sync_request;
	if (amIGroupLeader_ == true)
		sync_request = new SyncGroupRequest(topic_, group_, generation_id_, member_id_, member_partition_map_);
	else
		sync_request = new SyncGroupRequest(topic_, group_, generation_id_, member_id_);

	SendRequestHandler(coordinator_, sync_request);
	Response *response;
	ReceiveResponseHandler(coordinator_, &response);
	SyncGroupResponse *sync_response = dynamic_cast<SyncGroupResponse*>(response);
	int16_t error_code = sync_response->GetErrorCode();
	my_partitions_id_.clear();
	broker_owned_partition_.clear(); 
	if (error_code == ErrorCode::NO_ERROR)
	{
		sync_response->ParsePartitions(my_partitions_id_);
		broker_owned_partition_ = CreateBrokerIdToOwnedPartitionMap(my_partitions_id_);

		// next state
		current_state_ = &Network::PartOfGroup;
		event = Event::FETCH;
#if 0
		for (auto i = broker_owned_partition_.begin(); i != broker_owned_partition_.end(); ++i)
		{
			std::cout << "broker " << i->first << std::endl;
			for (auto j = i->second.begin(); j != i->second.end(); ++j)
			{
				std::cout << "partition " << *j << std::endl;
			}
		}
#endif
	}
	else if (error_code == ErrorCode::ILLEGAL_GENERATION || error_code == ErrorCode::UNKNOWN_MEMBER_ID)
	{
		// next state
		current_state_ = &Network::JoinGroup;
		event = Event::JOIN_WITH_EMPTY_CONSUMER_ID;
	}


	delete sync_request;
	delete sync_response;

	return 0;
}

int16_t Network::FetchValidOffset()
{
	int16_t error_code = 0;
	OffsetFetchRequest *offset_fetch_request = new OffsetFetchRequest(group_, topic_, my_partitions_id_);
	SendRequestHandler(coordinator_, offset_fetch_request);
	Response *response;
	ReceiveResponseHandler(coordinator_, &response);
	OffsetFetchResponse *offset_fetch_response = dynamic_cast<OffsetFetchResponse*>(response);
	offset_fetch_response->ParseOffset(partition_offset_);
	error_code = offset_fetch_response->GetErrorCode();
	if (error_code != ErrorCode::NO_ERROR)
	{
		delete offset_fetch_request;
		delete offset_fetch_response;
		return error_code;
	}

	delete offset_fetch_request;
	delete offset_fetch_response;

	for (auto po_it = partition_offset_.begin(); po_it != partition_offset_.end(); ++po_it)
	{
		if (po_it->second != -1)
			continue;

		std::vector<int> need_update_partitions;
		need_update_partitions.push_back(po_it->first);
		int leader_id = all_partitions_.at(po_it->first).GetLeaderId();
		Broker *leader = &brokers_.at(leader_id);

		OffsetRequest *offset_request = new OffsetRequest(topic_, need_update_partitions);
		SendRequestHandler(leader, offset_request);
		Response *response;
		ReceiveResponseHandler(leader, &response);
		OffsetResponse *offset_response = dynamic_cast<OffsetResponse*>(response);
		po_it->second = offset_response->GetNewOffset();
		delete offset_request;
		delete offset_response;

		int16_t tmp_error = CommitOffset(po_it->first, po_it->second);
		if (tmp_error != ErrorCode::NO_ERROR)
			error_code = tmp_error;
	}
	return error_code;
}

int16_t Network::CommitOffset(int32_t partition, int64_t offset)
{
	OffsetCommitRequest *commit_request = new OffsetCommitRequest(group_, generation_id_, member_id_, topic_, partition, offset);
	SendRequestHandler(coordinator_, commit_request);
	Response *r;
	ReceiveResponseHandler(coordinator_, &r);
	OffsetCommitResponse *commit_response = dynamic_cast<OffsetCommitResponse*>(r);
	int16_t error_code = commit_response->GetErrorCode();
	delete commit_request;
	delete r;
	return error_code;
}

int16_t Network::CommitOffset(const std::vector<PartitionOM> &partitions)
{
	OffsetCommitRequest *commit_request = new OffsetCommitRequest(group_, generation_id_, member_id_, topic_, partitions);
	SendRequestHandler(coordinator_, commit_request);
	Response *r;
	ReceiveResponseHandler(coordinator_, &r);
	OffsetCommitResponse *commit_response = dynamic_cast<OffsetCommitResponse*>(r);
	int16_t error_code = commit_response->GetErrorCode();
	delete commit_request;
	delete r;
	return error_code;
}

int Network::FetchMessage()
{
#if 0
	for (auto p_it = all_partitions_.begin(); p_it != all_partitions_.end(); ++p_it)
	{
		int partition = p_it->first;
		int64_t offset = partition_offset_[partition];
		Broker *leader = &brokers_.at(p_it->second.GetLeaderId());

		FetchRequest *fetch_request = new FetchRequest(topic_, partition, offset);
		SendRequestHandler(leader, fetch_request);
		Response *response;
		ReceiveResponseHandler(leader, &response);
		FetchResponse *fetch_response = dynamic_cast<FetchResponse*>(response);
		fetch_response->PrintTopicMsg();

		if (fetch_response->HasMessage(partition))
		{
			int64_t last_offset = fetch_response->GetLastOffset();
			CommitOffset(partition, last_offset + 1);
		}

		delete fetch_request;
		delete fetch_response;
	}
#endif

#if 1
	for (auto bp_it = broker_owned_partition_.begin(); bp_it != broker_owned_partition_.end(); ++bp_it)
	{
		std::vector<int> &owned_partitions = bp_it->second;
		std::vector<PartitionFM> fetch_partitions;
		for (auto p_it = owned_partitions.begin(); p_it != owned_partitions.end(); ++p_it)
		{
			int partition = *p_it;
			int64_t offset = partition_offset_[partition];
			fetch_partitions.push_back({partition, offset});
		}

		int leader_id = bp_it->first;
		Broker *leader = &brokers_.at(leader_id);
		FetchRequest *fetch_request = new FetchRequest(topic_, fetch_partitions);
		SendRequestHandler(leader, fetch_request);
		Response *response;
		ReceiveResponseHandler(leader, &response);
		FetchResponse *fetch_response = dynamic_cast<FetchResponse*>(response);
		fetch_response->PrintTopicMsg();

		std::vector<PartitionOM> commit_partitions;
		for (auto p_it = owned_partitions.begin(); p_it != owned_partitions.end(); ++p_it)
		{
			int partition = *p_it;
			int64_t offset = fetch_response->GetLastOffset(partition);
			if (offset < 0)
				continue;

			commit_partitions.push_back({partition, offset + 1});
		}

		if (!commit_partitions.empty())
			CommitOffset(commit_partitions);

		delete fetch_request;
		delete fetch_response;
	}
#endif

	return 0;
}

int Network::PartOfGroup(Event &event)
{
	switch(event)
	{
		case Event::FETCH:
		{
			int16_t error_code;
			error_code = FetchValidOffset();
			if (error_code == ErrorCode::ILLEGAL_GENERATION || error_code == ErrorCode::UNKNOWN_MEMBER_ID)
			{
				// next state
				current_state_ = &Network::JoinGroup;
				event = Event::JOIN_WITH_PREVIOUS_CONSUMER_ID;
				break;
			}

			FetchMessage();

			error_code = HeartbeatTask();
			if (error_code == ErrorCode::ILLEGAL_GENERATION || error_code == ErrorCode::UNKNOWN_MEMBER_ID)
			{
				// next state
				current_state_ = &Network::JoinGroup;
				event = Event::JOIN_WITH_PREVIOUS_CONSUMER_ID;
				break;
			}
			break;
		}
	}
	return 0;
}

int16_t Network::HeartbeatTask()
{
	Response *response;
	HeartbeatRequest *heart_request = new HeartbeatRequest(group_, generation_id_, member_id_);
	SendRequestHandler(coordinator_, heart_request);
	ReceiveResponseHandler(coordinator_, &response);
	HeartbeatResponse *heart_response = dynamic_cast<HeartbeatResponse*>(response);
	delete heart_request;
	delete heart_response;
	return heart_response->GetErrorCode();
}



