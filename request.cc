#include <iostream>
#include <arpa/inet.h>
#include <string.h>

#include "request.h"
#include "util.h"
#include "member_assignment.h"

//------------------------------Head
Request::Request(short api_key, int correlation_id, short api_version, std::string client_id)
{
	total_size_ = 0;
	api_key_ = api_key;
	api_version_ = api_version;
	correlation_id_ = correlation_id;
	client_id_ = client_id;
}

int Request::CountSize()
{
	return 2 + 2 + 4 + 2 + client_id_.length();
}

void Request::PrintAll()
{
	std::cout << "total size = " << total_size_ << std::endl;
	std::cout << "api key = " << api_key_ << std::endl;
	std::cout << "api version = " << api_version_ << std::endl;
	std::cout << "correlation id = " << correlation_id_ << std::endl;
	std::cout << "client id = " << client_id_ << std::endl;
}

int Request::Package(char **buf)
{
	// total size
	int request_size = htonl(total_size_);
	memcpy(*buf, &request_size, 4);
	(*buf) += 4;

	// api key
	short api_key = htons(api_key_);
	memcpy(*buf, &api_key, 2);
	(*buf) += 2;

	// api version
	short api_version = htons(api_version_);
	memcpy(*buf, &api_version, 2);
	(*buf) += 2;

	// correlation id
	int correlation_id = htonl(correlation_id_);
	memcpy(*buf, &correlation_id, 4);
	(*buf) += 4;

	// client id
	short client_id_size = htons((short)client_id_.length());
	memcpy(*buf, &client_id_size, 2);
	(*buf) += 2;
	memcpy(*buf, client_id_.c_str(), client_id_.length());
	(*buf) += client_id_.length();

	return 0;
}

//------------------------------GroupCoordinatorRequest
GroupCoordinatorRequest::GroupCoordinatorRequest(int correlation_id, const std::string &group_id)
	: Request(ApiKey::GroupCoordinatorRequest, correlation_id)
{
	group_id_ = group_id;
	total_size_ = CountSize();
}

int GroupCoordinatorRequest::CountSize()
{
	return Request::CountSize() +		// head
		   2 + group_id_.length();	// body
}

void GroupCoordinatorRequest::PrintAll()
{
	std::cout << "-----GroupCoordinatorRequest-----" << std::endl;
	Request::PrintAll();
	std::cout << "group id = " << group_id_ << std::endl;
	std::cout << "---------------------------------" << std::endl;
}

int GroupCoordinatorRequest::Package(char **buf)
{
	Request::Package(buf);

	// group id
	short group_id_size = htons((short)group_id_.length());
	memcpy(*buf, &group_id_size, 2);
	(*buf) += 2;
	memcpy(*buf, group_id_.c_str(), group_id_.length());

	return 0;
}

//------------------------------JoinGroupRequest
ProtocolMetadata::ProtocolMetadata(const std::vector<std::string> &topics)
{
	version_ = 0;
	subscription_ = topics;
	user_data_ = "";
}

int ProtocolMetadata::CountSize()
{
	int size = 0;
	size += 2;		// version
	size += 4;		// array
	for (unsigned int i = 0; i < subscription_.size(); i++)
		size += 2 + subscription_[i].size();
	size += 4 + user_data_.length();
	return size;
}

int ProtocolMetadata::Package(char **buf)
{
	// version
	short version = htons(version_);
	memcpy(*buf, &version, 2);
	(*buf) += 2;

	// topics
	int topics_size = htonl(subscription_.size());
	memcpy(*buf, &topics_size, 4);
	(*buf) += 4;

	for (auto sub_it = subscription_.begin(); sub_it != subscription_.end(); ++sub_it)
	{
		std::string &topic = *sub_it;
		short topic_size = htons((short)topic.length());
		memcpy(*buf, &topic_size, 2);
		(*buf) += 2;
		memcpy(*buf, topic.c_str(), topic.length());
		(*buf) += topic.length();
	}

	// user data
	int user_data_size = htonl(user_data_.size());
	memcpy(*buf, &user_data_size, 4);
	(*buf) += 4;
	memcpy(*buf, user_data_.data(), user_data_.size());

	return 0;
}

GroupProtocol::GroupProtocol(const std::vector<std::string> &topics)
	: protocol_metadata_(topics)
{
	assignment_strategy_ = "range";
}

int GroupProtocol::CountSize()
{
	return 2 + assignment_strategy_.length() +
		   4 /* bytes */ + protocol_metadata_.CountSize();
}

int GroupProtocol::Package(char **buf)
{
	// assignment strategy
	short assignment_strategy_size = htons((short)assignment_strategy_.length());
	memcpy(*buf, &assignment_strategy_size, 2);
	(*buf) += 2;
	memcpy(*buf, assignment_strategy_.c_str(), assignment_strategy_.length());
	(*buf) += assignment_strategy_.length();

	// ProtocolMetadata bytes size
	int protocol_metadata_size = htonl(protocol_metadata_.CountSize());
	memcpy(*buf, &protocol_metadata_size, 4);
	(*buf) += 4;

	protocol_metadata_.Package(buf);

	return 0;
}

JoinGroupRequest::JoinGroupRequest(int correlation_id,
		const std::string &group_id, const std::string member_id,
		const std::vector<std::string> &topics)
	: Request(ApiKey::JoinGroupRequest, correlation_id)
{
	group_id_ = group_id;
	session_timeout_ = 30000;
	member_id_ = member_id;
	protocol_type_ = "consumer";

	// only one group protocol
	GroupProtocol group_protocol(topics);
	group_protocols_.push_back(group_protocol);

	total_size_ = CountSize();
}

int JoinGroupRequest::CountSize()
{
	int size = Request::CountSize();		// head
	size +=	2 + group_id_.length() + 4 + 2 + member_id_.length() + 2 + protocol_type_.length() +
			4 /* array */;

	for (unsigned int i = 0; i < group_protocols_.size(); i++)
		size += group_protocols_[i].CountSize();

	return size;
}

void JoinGroupRequest::PrintAll()
{
	std::cout << "-----JoinGroupRequest-----" << std::endl;
	Request::PrintAll();
	std::cout << "group id = " << group_id_ << std::endl;
	std::cout << "session timeout = " << session_timeout_ << std::endl;
	std::cout << "member id = " << member_id_ << std::endl;
	std::cout << "protocol type = " << protocol_type_ << std::endl;

	for (unsigned int i = 0; i < group_protocols_.size(); i++)
	{
		GroupProtocol &gp = group_protocols_[i];
		std::cout << "	assignment strategy = " << gp.assignment_strategy_ << std::endl;
		std::cout << "	version = " << gp.protocol_metadata_.version_ << std::endl;
		for (unsigned int i = 0; i < gp.protocol_metadata_.subscription_.size(); i++)
			std::cout << "	subscription = " << gp.protocol_metadata_.subscription_[i] << std::endl;
		std::cout << "	user data = " << gp.protocol_metadata_.user_data_ << std::endl;
	}
	std::cout << "--------------------------" << std::endl;
}

int JoinGroupRequest::Package(char **buf)
{
	Request::Package(buf);

	// group id
	short group_id_size = htons((short)group_id_.length());
	memcpy(*buf, &group_id_size, 2);
	(*buf) += 2;
	memcpy(*buf, group_id_.c_str(), group_id_.length());
	(*buf) += group_id_.length();

	// session timeout
	int session_timeout = htonl(session_timeout_);
	memcpy(*buf, &session_timeout, 4);
	(*buf) += 4;

	// member id
	short member_id_size = htons((short)member_id_.length());
	memcpy(*buf, &member_id_size, 2);
	(*buf) += 2;
	memcpy(*buf, member_id_.c_str(), member_id_.length());
	(*buf) += member_id_.length();

	// protocol type
	short protocol_type_size = htons((short)protocol_type_.length());
	memcpy(*buf, &protocol_type_size, 2);
	(*buf) += 2;
	memcpy(*buf, protocol_type_.c_str(), protocol_type_.length());
	(*buf) += protocol_type_.length();

	// array size
	int group_protocols_size = htonl(group_protocols_.size());
	memcpy(*buf, &group_protocols_size, 4);
	(*buf) += 4;

	for (auto gp_it = group_protocols_.begin(); gp_it != group_protocols_.end(); ++gp_it)
	{
		gp_it->Package(buf);
	}

	return 0;
}

//------------------------------MetadataRequest
MetadataRequest::MetadataRequest(int correlation_id, const std::vector<std::string> &topic_names,
		bool for_all_topic)
	: Request(ApiKey::MetadataRequest, correlation_id)
{
	if (for_all_topic == false)
		topic_names_ = topic_names;
	total_size_ = CountSize();
}

int MetadataRequest::CountSize()
{
	int size = Request::CountSize();
	size += 4;
	for (unsigned int i = 0; i < topic_names_.size(); i++)
	{
		size += 2 + topic_names_[i].length();
	}

	return size;
}

void MetadataRequest::PrintAll()
{
	std::cout << "-----MetadataRequest-----" << std::endl;
	Request::PrintAll();
	for (unsigned int i = 0; i < topic_names_.size(); i++)
	{
		std::cout << "topic name = " << topic_names_[i] << std::endl;
	}
	std::cout << "-------------------------" << std::endl;
}

int MetadataRequest::Package(char **buf)
{
	Request::Package(buf);

	// topics array
	int topic_names_size = htonl(topic_names_.size());
	memcpy(*buf, &topic_names_size, 4);
	(*buf) += 4;

	for (auto tn_it = topic_names_.begin(); tn_it != topic_names_.end(); ++tn_it)
	{
		std::string &topic = *tn_it;
		short topic_size = htons((short)topic.length());
		memcpy(*buf, &topic_size, 2);
		(*buf) += 2;
		memcpy(*buf, topic.c_str(), topic.length());
		(*buf) += topic.length();
	}

	return 0;
}
//------------------------------SyncGroupRequest
GroupAssignment::GroupAssignment(const std::string &topic, const std::string &member_id,
		const std::vector<int> &partitions)
	: member_assignment_(topic, partitions)
{
	member_id_ = member_id;
}

int GroupAssignment::CountSize()
{
	int size = 0;
	size += 2 + member_id_.length() +
			4 + member_assignment_.CountSize();

	return size;
}

void GroupAssignment::PrintAll()
{
	std::cout << "member id = " << member_id_ << std::endl;
	member_assignment_.PrintAll();
}

int GroupAssignment::Package(char **buf)
{
	// member id
	short member_id_size = htons((short)member_id_.length());
	memcpy(*buf, &member_id_size, 2);
	(*buf) += 2;
	memcpy(*buf, member_id_.c_str(), member_id_.length());
	(*buf) += member_id_.length();

	// MemberAssignment bytes
	int member_assignment_size = htonl(member_assignment_.CountSize());
	memcpy(*buf, &member_assignment_size, 4);
	(*buf) += 4;
	member_assignment_.Package(buf);

	return 0;
}

SyncGroupRequest::SyncGroupRequest(int correlation_id, const std::string &topic, const std::string group_id,
		int generation_id, const std::string &member_id,
		const std::map<std::string, std::vector<int>> &member_partition_map)
	: Request(ApiKey::SyncGroupRequest, correlation_id)
{
	group_id_ = group_id;
	generation_id_ = generation_id;
	member_id_ = member_id;

	for (auto mp_it = member_partition_map.begin(); mp_it != member_partition_map.end(); ++mp_it)
	{
		GroupAssignment group_assignment(topic, mp_it->first, mp_it->second);
		group_assignment_.push_back(group_assignment);
	}

	total_size_ = CountSize();
}

int SyncGroupRequest::CountSize()
{
	int size = Request::CountSize();
	size += 2 + group_id_.length() +	// group id
			4 +		// generation id
			2 + member_id_.length(); 	// member id

	// GroupAssignment array
	size += 4;
	for (auto ga_it = group_assignment_.begin(); ga_it != group_assignment_.end(); ++ga_it)
	{
		size += ga_it->CountSize();
	}

	return size;
}

void SyncGroupRequest::PrintAll()
{
	std::cout << "-----SyncGroupRequest-----" << std::endl;
	Request::PrintAll();
	std::cout << "group id = " << group_id_ << std::endl;
	std::cout << "generation id = " << generation_id_ << std::endl;
	std::cout << "member id = " << member_id_ << std::endl;
	for (auto ga_it = group_assignment_.begin(); ga_it != group_assignment_.end(); ++ga_it)
	{
		ga_it->PrintAll();
	}
	std::cout << "-------------------------" << std::endl;
}

int SyncGroupRequest::Package(char **buf)
{
	Request::Package(buf);

	// group id
	short group_id_size = htons((short)group_id_.length());
	memcpy(*buf, &group_id_size, 2);
	(*buf) += 2;
	memcpy(*buf, group_id_.c_str(), group_id_.length());
	(*buf) += group_id_.length();

	// generation id
	int generation_id = htonl(generation_id_);
	memcpy(*buf, &generation_id, 4);
	(*buf) += 4;

	// member id
	short member_id_size = htons((short)member_id_.length());
	memcpy(*buf, &member_id_size, 2);
	(*buf) += 2;
	memcpy(*buf, member_id_.c_str(), member_id_.length());
	(*buf) += member_id_.length();

	// group assignment array
	int group_assignment_size = htonl(group_assignment_.size());
	memcpy(*buf, &group_assignment_size, 4);
	(*buf) += 4;

	for (auto ga_it = group_assignment_.begin(); ga_it != group_assignment_.end(); ++ga_it)
	{
		ga_it->Package(buf);
	}

	return 0;
}

//------------------------------HeartbeatRequest
HeartbeatRequest::HeartbeatRequest(int correlation_id, const std::string &group_id,
		int generation_id, const std::string &member_id)
	: Request(ApiKey::HeartbeatRequest, correlation_id)
{
	group_id_ = group_id;
	generation_id_ = generation_id;
	member_id_ = member_id;

	total_size_ = CountSize();
}

int HeartbeatRequest::CountSize()
{
	int size = Request::CountSize();
	size += 2 + group_id_.length()+
			4 +
			2 + member_id_.length();

	return size;
}

void HeartbeatRequest::PrintAll()
{
	std::cout << "-----HeartbeatRequest-----" << std::endl;
	Request::PrintAll();
	std::cout << "group id = " << group_id_ << std::endl;
	std::cout << "generation id = " << generation_id_ << std::endl;
	std::cout << "member id = " << member_id_ << std::endl;
	std::cout << "-------------------------" << std::endl;
}

int HeartbeatRequest::Package(char **buf)
{
	Request::Package(buf);

	// group id
	short group_id_size = htons((short)group_id_.length());
	memcpy(*buf, &group_id_size, 2);
	(*buf) += 2;
	memcpy(*buf, group_id_.c_str(), group_id_.length());
	(*buf) += group_id_.length();

	// generation id
	int generation_id = htonl(generation_id_);
	memcpy(*buf, &generation_id, 4);
	(*buf) += 4;

	// member id
	short member_id_size = htons((short)member_id_.length());
	memcpy(*buf, &member_id_size, 2);
	(*buf) += 2;
	memcpy(*buf, member_id_.c_str(), member_id_.length());

	return 0;
}

//------------------------------FetchRequest
FetchRequest::FetchRequest(int correlation_id, const std::string &topic_name, int partition, long fetch_offset)
	: Request(ApiKey::FetchRequest, correlation_id, 1)
{
	replica_id_ = -1;
	max_wait_time_ = 500;
	min_bytes_ = 1024;
	topic_name_ = topic_name;
	partition_ = partition;
	fetch_offset_ = fetch_offset;
	max_bytes_ = 1048576;

	total_size_ = CountSize();
}

int FetchRequest::CountSize()
{
	int size = Request::CountSize();
	size += 4 + 4 + 4 +
		    4 + 2 + topic_name_.length() +
		    4 + 4 + 8 + 4;
	return size;
}

void FetchRequest::PrintAll()
{
	std::cout << "-----FetchRequest-----" << std::endl;
	Request::PrintAll();
	std::cout << "replica id = " << replica_id_ << std::endl;
	std::cout << "max wait time = " << max_wait_time_ << std::endl;
	std::cout << "min bytes = " << min_bytes_ << std::endl;
	std::cout << "topic name = " << topic_name_ << std::endl;
	std::cout << "partition = " << partition_ << std::endl;
	std::cout << "fetch offset = " << fetch_offset_ << std::endl;
	std::cout << "max bytes = " << max_bytes_ << std::endl;
	std::cout << "----------------------" << std::endl;
}

int FetchRequest::Package(char **buf)
{
	Request::Package(buf);

	// replica id
	int replica_id = htonl(replica_id_);
	memcpy(*buf, &replica_id, 4);
	(*buf) += 4;

	// max wait time
	int max_wait_time = htonl(max_wait_time_);
	memcpy(*buf, &max_wait_time, 4);
	(*buf) += 4;

	// min bytes
	int min_bytes = htonl(min_bytes_);
	memcpy(*buf, &min_bytes, 4);
	(*buf) += 4;

	// array size
	int topic_partition_size = htonl(1);
	memcpy(*buf, &topic_partition_size, 4);
	(*buf) += 4;

	// topic name
	short topic_name_size = htons((short)topic_name_.length());
	memcpy(*buf, &topic_name_size, 2);
	(*buf) += 2;
	memcpy(*buf, topic_name_.c_str(), topic_name_.length());
	(*buf) += topic_name_.length();

	// array size
	int partition_size = htonl(1);
	memcpy(*buf, &partition_size, 4);
	(*buf) += 4;

	// partition
	int partition = htonl(partition_);
	memcpy(*buf, &partition, 4);
	(*buf) += 4;

	// min bytes
	long fetch_offset = htobe64(fetch_offset_);
	memcpy(*buf, &fetch_offset, 8);
	(*buf) += 8;

	// max bytes 
	int max_bytes = htonl(max_bytes_);
	memcpy(*buf, &max_bytes, 4);
	(*buf) += 4;

	return 0;
}

//------------------------------OffsetFetchRequest
OffsetFetchRequest::OffsetFetchRequest(int correlation_id, const std::string &group,
			const std::string &topic, const std::vector<int> &partitions)
	: Request(ApiKey::OffsetFetchRequest, correlation_id, 1)
{
	group_ = group;
	topic_ = topic;
	partitions_ = partitions;
	total_size_ = CountSize();
}

int OffsetFetchRequest::CountSize()
{
	int size = Request::CountSize();
	size += 2 + group_.length();
	size += 4 + 2 + topic_.length();
	size += 4 + 4 * partitions_.size();
	return size;
}

void OffsetFetchRequest::PrintAll()
{
	std::cout << "-----OffsetFetchRequest-----" << std::endl;
	Request::PrintAll();
	std::cout << "group = " << group_ << std::endl;
	std::cout << "topic = " << topic_ << std::endl;
	for (auto p_it = partitions_.begin(); p_it != partitions_.end(); ++p_it)
	{
		std::cout << "partition = " << *p_it << std::endl;
	}
	std::cout << "----------------------------" << std::endl;

}

int OffsetFetchRequest::Package(char **buf)
{
	Request::Package(buf);

	// group
	short group_size = htons((short)group_.length());
	memcpy(*buf, &group_size, 2);
	(*buf) += 2;
	memcpy(*buf, group_.c_str(), group_.length());
	(*buf) += group_.length();

	// topic array size
	int topic_partition_size = htonl(1);
	memcpy(*buf, &topic_partition_size, 4);
	(*buf) += 4;

	// topic
	short topic_length = htons((short)topic_.length());
	memcpy(*buf, &topic_length, 2);
	(*buf) += 2;
	memcpy(*buf, topic_.c_str(), topic_.length());
	(*buf) += topic_.length();

	// topic array size
	int partition_size = htonl(partitions_.size());
	memcpy(*buf, &partition_size, 4);
	(*buf) += 4;

	for (auto p_it = partitions_.begin(); p_it != partitions_.end(); ++p_it)
	{
		int partition = htonl(*p_it);
		memcpy(*buf, &partition, 4);
		(*buf) += 4;
	}

	return 0;
}


