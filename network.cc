#include <iostream>
#include <unistd.h>
#include <deque>
#include <string.h>

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
		if (fd < 0)
			continue;
		Broker broker(fd, tmp_broker_id, ip, port);
		//seed_brokers_.push_back(broker);
		alive_brokers_.insert({tmp_broker_id, broker});
		tmp_broker_id--;
	}

	//pthread_mutex_init(&queue_mutex_, NULL);

	event_ = Event::STARTUP;
	current_state_ = &Network::Initial;
}

Network::~Network()
{
	// close socket and delete brokers
	for (auto it = alive_brokers_.begin(); it != alive_brokers_.end(); ++it)
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

// return value:
// >0: OK, remember delete response
// =0: peer down, remove it from alive_brokers_
// <0: ERROR
int Network::ReceiveResponseHandler(Broker *broker, Response **response)
{
	int ret = Receive(broker->fd_, response);
	if (ret > 0)
		(*response)->PrintAll();
	else if (ret == 0)
	{
		for (auto b_it = alive_brokers_.begin(); b_it != alive_brokers_.end(); /* NULL */)
		{
			if (&(b_it->second) != broker)
			{
				++b_it;
				continue;
			}
			close(b_it->second.fd_);
			b_it = alive_brokers_.erase(b_it);
			break;
		}
	}
	return ret;
}

// return value:
// >0 : OK
// =0 : close by peer, remove it from alive_brokers_
// <0 : ERROR, remove it from alive_brokers_
int Network::SendRequestHandler(Broker *broker, Request *request)
{
	request->PrintAll();
	int ret = Send(broker->fd_, request);
	if (ret > 0)
	{
		last_correlation_id_ = request->GetCorrelationId();
		last_api_key_ = request->GetApiKey();
	}
	else if (ret <= 0)
	{
		for (auto b_it = alive_brokers_.begin(); b_it != alive_brokers_.end(); /* NULL */)
		{
			if (&(b_it->second) != broker)
			{
				++b_it;
				continue;
			}
			close(b_it->second.fd_);
			b_it = alive_brokers_.erase(b_it);
			break;
		}
	}

	return ret;
}

// return value:
// >0: OK, remember delete res
// =0: peer down
// <0: ERROR
int Network::Receive(int fd, Response **res)
{
	char size_buf[4];
	int nread = read(fd, size_buf, 4);
	if (nread != 4)
	{
		LOG(INFO) << "Response header total len = 0, broker down";
		return 0;
	}
	int total_len = Util::NetBytesToInt(size_buf);
	LOG(DEBUG) << "Response header total len = " << total_len;

	char *buf = new char[total_len + 4];
	memcpy(buf, size_buf, 4);

	if (CompleteRead(fd ,buf + 4, total_len) <= 0)
	{
		delete[] buf;
		return -1;
	}

	//int response_size = Util::NetBytesToInt(buf);
	int correlation_id = Util::NetBytesToInt(buf + 4);
	int api_key = GetApiKeyFromResponse(correlation_id);
	if (api_key < 0)
	{
		// not match
		delete[] buf;
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
			try
			{
				SyncGroupResponse *response = new SyncGroupResponse(&p);
				*res = response;
			}
			catch (...)
			{
				std::cout << "sync group response error" << std::endl;
				return -1;
			}
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
	return 1;
}

// return value:
// >0 : OK
// =0 : close by peer
// <0 : ERROR
int Network::Send(int fd, Request *request)
{
	int packet_size = request->CountSize() + 4;
	char buf[2048];
	char *p = buf;

	request->Package(&p);

	int nwrite = write(fd, buf, packet_size);
	LOG(DEBUG) << "Send " << nwrite << " bytes";
	if (nwrite == 0)
		LOG(INFO) << "Disconected by peer...";
	else if (nwrite < 0)
		LOG(ERROR) << "Send request failed, return value = " << nwrite;

	return nwrite;
}

short Network::GetApiKeyFromResponse(int correlation_id)
{
	if (last_correlation_id_ != correlation_id)
	{
		std::cerr << "The correlation_id are not equal" << std::endl;
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
		//member_partitions_.insert({*m_it, owned});
		member_partitions_[*m_it] = owned;
	}
	return 0;
}

// return value:
// >0: OK
// =0: close by peer
// <0: ERROR
int Network::CompleteRead(int fd, char *buf, int total_len)
{
	int sum_read = 0;
	while (sum_read != total_len)
	{
		char tmp_buf[1048576] = {0};
		int nread = read(fd, tmp_buf, sizeof(tmp_buf));
		LOG(DEBUG) << "nread = " << nread;

		if (nread <= 0)
		{
			if (nread == 0)
			{
				LOG(INFO) << "connection has been closed";
				std::cerr << "connection has been closed" << std::endl;
			}
			else
			{
				LOG(ERROR) << "error occurred";
				std::cerr << "error occurred" << std::endl;
			}

			return nread;
		}

		memcpy(buf + 0 + sum_read, tmp_buf, nread);
		sum_read += nread;
	}
	return sum_read;
}

// Connect new broker if we found it
void Network::ConnectNewBroker(std::unordered_map<int, Broker> &brokers)
{
	for (auto b_it = brokers.begin(); b_it != brokers.end(); ++b_it)
	{
		Broker &broker = b_it->second;
		if (broker.fd_ > 0)
			continue;

		std::string ip = broker.ip_;
		int port = broker.port_;
		int fd = NetUtil::NewTcpClient(ip.c_str(), port);
		if (fd < 0)
		{
			// This may happen when broker down and metadata response contain this broker
			// in first few seconds.
			continue;
		}

		broker.fd_ = fd;
	}
}

// In old, but not in new, desconnect
void Network::DisconnectDownBroker(std::unordered_map<int, Broker> &old_brokers, std::unordered_map<int, Broker> &new_brokers)
{
	for (auto b_it = old_brokers.begin(); b_it != old_brokers.end(); ++b_it)
	{
		int old_bid = b_it->first;
		if (old_bid < 0 || new_brokers.find(old_bid) != new_brokers.end())
			continue;
		close(b_it->second.fd_);
	}
}

int Network::GetFdFromIp(const std::string &alive_ip, const std::unordered_map<int, Broker> &alive_brokers)
{
	for (auto b_it = alive_brokers.begin(); b_it != alive_brokers.end(); ++b_it)
	{
		const Broker &b = b_it->second;
		if (b.ip_ == alive_ip)
			return b.fd_;
	}
	return -1;
}

void Network::RefreshAliveBrokers(std::unordered_map<int, Broker> &alive_brokers, std::unordered_map<int, Broker> &updated_brokers)
{
	for (auto b_it = updated_brokers.begin(); b_it != updated_brokers.end(); ++b_it)
	{
		b_it->second.ip_ = Util::HostnameToIp(b_it->second.ip_);
		int fd = GetFdFromIp(b_it->second.ip_, alive_brokers);
		if (fd > 0)
			b_it->second.fd_ = fd;
	}
	ConnectNewBroker(updated_brokers);
	DisconnectDownBroker(alive_brokers, updated_brokers);
	alive_brokers = updated_brokers;
}

// return value:
// >0: OK, remember delete meta_response
// <0: send or receive error
int Network::FetchMetadata(MetadataRequest *meta_request, MetadataResponse **meta_response)
{
	for (auto b_it = alive_brokers_.begin(); b_it != alive_brokers_.end(); /* NULL */)
	{
		Broker *broker = &(b_it->second);
		if (SendRequestHandler(broker, meta_request) <= 0)
		{
			b_it = alive_brokers_.begin();
			continue;
		}

		Response *response;
		if (ReceiveResponseHandler(broker, &response) <= 0)
		{
			b_it = alive_brokers_.begin();
			continue;
		}

		*meta_response = dynamic_cast<MetadataResponse*>(response);
		return 1;
	}
	LOG(ERROR) << "Fetch metadata failed...";
	return -1;
}


//---------------------------state functions
int Network::Initial(Event &event)
{
	switch(event)
	{
		case Event::STARTUP:
		{
			std::vector<std::string> topics({topic_});
			MetadataRequest *meta_request = new MetadataRequest(topics);
			MetadataResponse *meta_response;
			if (FetchMetadata(meta_request, &meta_response) < 0)
			{
				delete meta_request;
				break;
			}

			std::unordered_map<int, Broker> updated_brokers;
		    meta_response->ParseBrokers(updated_brokers);
			if (!updated_brokers.empty())
			{
				RefreshAliveBrokers(alive_brokers_, updated_brokers);
				if (meta_response->ParsePartitions(all_partitions_) == ErrorCode::NO_ERROR)
				{
					// next state
					current_state_ = &Network::DiscoverCoordinator;
					event = Event::DISCOVER_COORDINATOR;
				}
			}
			else
			{
				// No valid broker, sleep and retry metadata request in next loop
				sleep(5);
			}

			delete meta_request;
			delete meta_response;
			break;
		}
		case Event::REFRESH_METADATA:
		{
			std::vector<std::string> topics({topic_});
			MetadataRequest *meta_request = new MetadataRequest(topics);
			MetadataResponse *meta_response;
			if (FetchMetadata(meta_request, &meta_response) < 0)
			{
				delete meta_request;
				break;
			}

			std::unordered_map<int, Broker> updated_brokers;
		    meta_response->ParseBrokers(updated_brokers);
			if (!updated_brokers.empty())
			{
				RefreshAliveBrokers(alive_brokers_, updated_brokers);
				if (alive_brokers_.find(coordinator_->id_) != alive_brokers_.end())
				{
					// coordinator still up
					coordinator_ = &(alive_brokers_.find(coordinator_->id_)->second);
					if (meta_response->ParsePartitions(all_partitions_) == ErrorCode::NO_ERROR)
					{
						// next state
						current_state_ = &Network::JoinGroup;
						event = Event::JOIN_WITH_PREVIOUS_CONSUMER_ID;
					}
				}
				else
				{
					// coordinator down, we should discover it.
					current_state_ = &Network::DiscoverCoordinator;
					event = Event::DISCOVER_COORDINATOR;
				}
			}
			else
			{
				// No valid broker, sleep and retry metadata request in next loop
				sleep(5);
			}

			delete meta_request;
			delete meta_response;
			break;
		}
	}
	return 0;
}

int Network::DiscoverCoordinator(Event &event)
{
	// Select the first broker
	auto b_it = alive_brokers_.begin();
	Broker *broker = &(b_it->second);

	GroupCoordinatorRequest *group_request = new GroupCoordinatorRequest(group_);
	if (SendRequestHandler(broker, group_request) <= 0)
	{
		// next state
		current_state_ = &Network::Initial;
		event = Event::STARTUP;
		LOG(INFO) << "Change state to Initial...";
		delete group_request;
		return 0;
	}

	Response *response;
	if (ReceiveResponseHandler(broker, &response) <= 0)
	{
		// next state
		current_state_ = &Network::Initial;
		event = Event::STARTUP;
		LOG(INFO) << "Change state to Initial...";
		delete group_request;
		return 0;
	}

	GroupCoordinatorResponse *group_response = dynamic_cast<GroupCoordinatorResponse*>(response);

	int32_t co_id = group_response->GetCoordinatorId();
	coordinator_ = &alive_brokers_.at(co_id);

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
			if (SendRequestHandler(coordinator_, join_request) <= 0)
			{
				delete join_request;
				break;
			}

			Response *response;
			if (ReceiveResponseHandler(coordinator_, &response) <= 0)
			{
				delete join_request;
				break;
			}
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
			if (SendRequestHandler(coordinator_, join_request) <= 0)
			{
				delete join_request;
				break;
			}
			Response *response;
			if (ReceiveResponseHandler(coordinator_, &response) <= 0)
			{
				delete join_request;
				break;
			}
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
std::unordered_map<int, std::vector<int>> Network::CreateBrokerIdToOwnedPartitionMap(const std::vector<int> &owned_partitions)
{
	std::unordered_map<int, std::vector<int>> result;
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
		sync_request = new SyncGroupRequest(topic_, group_, generation_id_, member_id_, member_partitions_);
	else
		sync_request = new SyncGroupRequest(topic_, group_, generation_id_, member_id_);

	if (SendRequestHandler(coordinator_, sync_request) <= 0)
	{
		delete sync_request;
		return -1;
	}
	Response *response;
	if (ReceiveResponseHandler(coordinator_, &response) <= 0)
	{
		// next state
		current_state_ = &Network::JoinGroup;
		event = Event::JOIN_WITH_EMPTY_CONSUMER_ID;

		delete sync_request;
		return -1;
	}
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

// return value:
// <0: send or receive error
// =0: OK
// >0: some error code in response
int16_t Network::FetchValidOffset()
{
	int16_t error_code = 0;
	OffsetFetchRequest *offset_fetch_request = new OffsetFetchRequest(group_, topic_, my_partitions_id_);
	if (SendRequestHandler(coordinator_, offset_fetch_request) <= 0)
	{
		delete offset_fetch_request;
		return -1;
	}

	Response *response;
	if ((error_code = ReceiveResponseHandler(coordinator_, &response)) <= 0)
	{
		delete offset_fetch_request;
		return -1;
	}
	OffsetFetchResponse *offset_fetch_response = dynamic_cast<OffsetFetchResponse*>(response);
	if (offset_fetch_response->GetErrorCode() != ErrorCode::NO_ERROR)
	{
		delete offset_fetch_request;
		delete offset_fetch_response;
		return error_code;
	}
	offset_fetch_response->ParseOffset(partition_offset_);

	delete offset_fetch_request;
	delete offset_fetch_response;

	for (auto po_it = partition_offset_.begin(); po_it != partition_offset_.end(); ++po_it)
	{
		if (po_it->second != -1)
			continue;

		std::vector<int> need_update_partitions;
		need_update_partitions.push_back(po_it->first);
		int leader_id = all_partitions_.at(po_it->first).GetLeaderId();
		std::cout << "leader id = " << leader_id << std::endl;
		Broker *leader = &alive_brokers_.at(leader_id);

		OffsetRequest *offset_request = new OffsetRequest(topic_, need_update_partitions);
		if (SendRequestHandler(leader, offset_request) <= 0)
		{
			delete offset_request;
			continue;
		}
		Response *response;
		if (ReceiveResponseHandler(leader, &response) <= 0)
		{
			delete offset_request;
			continue;
		}
		OffsetResponse *offset_response = dynamic_cast<OffsetResponse*>(response);
		po_it->second = offset_response->GetNewOffset();
		delete offset_request;
		delete offset_response;

		int16_t tmp_error = CommitOffset(po_it->first, po_it->second);
		if (tmp_error != ErrorCode::NO_ERROR)
			error_code = tmp_error;
		else
			return -1;
	}
	return error_code;
}

// return value:
// <0: send or receive error
// =0: OK
// >0: some error code in response
int16_t Network::CommitOffset(int32_t partition, int64_t offset)
{
	OffsetCommitRequest *commit_request = new OffsetCommitRequest(group_, generation_id_, member_id_, topic_, partition, offset);
	if (SendRequestHandler(coordinator_, commit_request) <= 0)
	{
		delete commit_request;
		return -1;
	}
	Response *r;
	if (ReceiveResponseHandler(coordinator_, &r) <= 0)
	{
		delete commit_request;
		return -1;
	}
	OffsetCommitResponse *commit_response = dynamic_cast<OffsetCommitResponse*>(r);
	int16_t error_code = commit_response->GetErrorCode();
	delete commit_request;
	delete r;
	return error_code;
}

// return value:
// <0: send or receive error
// =0: OK
// >0: some error code in response
int16_t Network::CommitOffset(const std::vector<PartitionOM> &partitions)
{
	OffsetCommitRequest *commit_request = new OffsetCommitRequest(group_, generation_id_, member_id_, topic_, partitions);
	if (SendRequestHandler(coordinator_, commit_request) <= 0)
	{
		delete commit_request;
		return -1;
	}

	Response *r;
	if (ReceiveResponseHandler(coordinator_, &r) <= 0)
	{
		delete commit_request;
		return -1;
	}
	OffsetCommitResponse *commit_response = dynamic_cast<OffsetCommitResponse*>(r);
	int16_t error_code = commit_response->GetErrorCode();
	delete commit_request;
	delete r;
	return error_code;
}

// return value:
// =0 : OK
// <0 : ERROR
int Network::FetchMessage()
{
	for (auto bp_it = broker_owned_partition_.begin(); bp_it != broker_owned_partition_.end(); ++bp_it)
	{
		std::vector<int> &owned_partitions = bp_it->second;
		std::vector<PartitionFM> fetch_partitions;
		for (auto p_it = owned_partitions.begin(); p_it != owned_partitions.end(); ++p_it)
		{
			int partition = *p_it;
			int64_t offset = partition_offset_.at(partition);
			if (offset < 0)
				continue;
			fetch_partitions.push_back({partition, offset});
		}

		int leader_id = bp_it->first;
		Broker *leader = &alive_brokers_.at(leader_id);
		FetchRequest *fetch_request = new FetchRequest(topic_, fetch_partitions);
		if (SendRequestHandler(leader, fetch_request) <= 0)
		{
			delete fetch_request;
			return -1;
		}

		Response *response;
		if (ReceiveResponseHandler(leader, &response) <= 0)
		{
			delete fetch_request;
			return -1;
		}

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
		{
			if (CommitOffset(commit_partitions) != ErrorCode::NO_ERROR)
			{
				delete fetch_request;
				delete fetch_response;
				return -1;
			}
		}

		delete fetch_request;
		delete fetch_response;
	}
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
				LOG(INFO) << "Change state to JoinGroup...";
				break;
			}
			else if (error_code < 0)
			{
				// next state
				current_state_ = &Network::Initial;
				event = Event::REFRESH_METADATA;
				LOG(INFO) << "Change state to Initial...";
				break;
			}

			int ret = FetchMessage();
			if (ret < 0)
			{
				// next state
				current_state_ = &Network::Initial;
				event = Event::STARTUP;
				LOG(INFO) << "Change state to Initial...";
				break;
			}

			error_code = HeartbeatTask();
			if (error_code == ErrorCode::ILLEGAL_GENERATION || error_code == ErrorCode::UNKNOWN_MEMBER_ID)
			{
				// next state
				current_state_ = &Network::JoinGroup;
				event = Event::JOIN_WITH_PREVIOUS_CONSUMER_ID;
				LOG(INFO) << "Change state to JoinGroup...";
				break;
			}
			else if (error_code < 0)
			{
				// next state
				current_state_ = &Network::Initial;
				event = Event::REFRESH_METADATA;
				LOG(INFO) << "Change state to Initial...";
				break;
			}

			// next state
			current_state_ = &Network::Initial;
			event = Event::REFRESH_METADATA;
			break;
		}
	}
	return 0;
}

// return value:
// -1: send or receive error
// =0: OK
// >0: there is a error code in response
int16_t Network::HeartbeatTask()
{
	Response *response;
	HeartbeatRequest *heart_request = new HeartbeatRequest(group_, generation_id_, member_id_);
	if (SendRequestHandler(coordinator_, heart_request) <= 0)
	{
		delete heart_request;
		return -1;
	}
	if (ReceiveResponseHandler(coordinator_, &response) <= 0)
	{
		delete heart_request;
		return -1;
	}
	HeartbeatResponse *heart_response = dynamic_cast<HeartbeatResponse*>(response);
	delete heart_request;
	delete heart_response;
	return heart_response->GetErrorCode();
}



