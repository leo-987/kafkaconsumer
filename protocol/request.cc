#include <iostream>
#include <arpa/inet.h>
#include <string.h>

#include "request.h"
#include "member_assignment.h"
#include "request_response_type.h"

//------------------------------Head
Request::Request(short api_key, int correlation_id, short api_version, std::string client_id)
{
	total_size_ = 0;
	api_key_ = api_key;
	api_version_ = api_version;
	correlation_id_ = correlation_id;
	client_id_ = client_id;
}

int Request::GetCorrelationId()
{
	return correlation_id_;
}

short Request::GetApiKey()
{
	return api_key_;
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
	: Request(ApiKey::GroupCoordinatorType, correlation_id)
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
	: Request(ApiKey::JoinGroupType, correlation_id)
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


