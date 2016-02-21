#include <iostream>

#include "request.h"

//------------------------------Head
Request::Request(short api_key, int correlation_id, std::string client_id)
{
	total_size_ = 0;
	api_key_ = api_key;
	api_version_ = 0;
	correlation_id_ = correlation_id;
	client_id_ = client_id;
}

int Request::Size()
{
	return 2 + 2 + 4 + 2 + client_id_.length();
}

void Request::Print()
{
	std::cout << "total size = " << total_size_ << std::endl;
	std::cout << "api key = " << api_key_ << std::endl;
	std::cout << "api version = " << api_version_ << std::endl;
	std::cout << "correlation id = " << correlation_id_ << std::endl;
	std::cout << "client id = " << client_id_ << std::endl;
}

//------------------------------GroupCoordinatorRequest
GroupCoordinatorRequest::GroupCoordinatorRequest(int correlation_id, const std::string &group_id)
	: Request(ApiKey::GroupCoordinatorRequest, correlation_id)
{
	group_id_ = group_id;
	total_size_ = Size();
}

int GroupCoordinatorRequest::Size()
{
	return Request::Size() +		// head
		   2 + group_id_.length();	// body
}

void GroupCoordinatorRequest::Print()
{
	std::cout << "-----GroupCoordinatorRequest-----" << std::endl;
	Request::Print();
	std::cout << "group id = " << group_id_ << std::endl;
	std::cout << "---------------------------------" << std::endl;
}

//------------------------------JoinGroupRequest
ProtocolMetadata::ProtocolMetadata(const std::vector<std::string> &topics)
{
	version_ = 0;
	subscription_ = topics;
	user_data_ = "";
}

int ProtocolMetadata::Size()
{
	int size = 0;
	size += 2;		// version
	size += 4;		// array
	for (unsigned int i = 0; i < subscription_.size(); i++)
		size += 2 + subscription_[i].size();
	size += 4 + user_data_.length();
	return size;
}

GroupProtocol::GroupProtocol(const std::vector<std::string> &topics)
	: protocol_metadata_(topics)
{
	assignment_strategy_ = "range";
}

int GroupProtocol::Size()
{
	return 2 + assignment_strategy_.length() +
		   4 /* bytes */ + protocol_metadata_.Size();
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

	total_size_ = Size();
}

int JoinGroupRequest::Size()
{
	int size = Request::Size();		// head
	size +=	2 + group_id_.length() + 4 + 2 + member_id_.length() + 2 + protocol_type_.length() +
			4 /* array */;

	for (unsigned int i = 0; i < group_protocols_.size(); i++)
		size += group_protocols_[i].Size();

	return size;
}

void JoinGroupRequest::Print()
{
	std::cout << "-----JoinGroupRequest-----" << std::endl;
	Request::Print();
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

//------------------------------MetadataRequest
MetadataRequest::MetadataRequest(int correlation_id, const std::vector<std::string> &topic_names)
	: Request(ApiKey::MetadataRequest, correlation_id)
{
	topic_names_ = topic_names;
	total_size_ = Size();
}

int MetadataRequest::Size()
{
	int size = Request::Size();
	size += 4;
	for (unsigned int i = 0; i < topic_names_.size(); i++)
	{
		size += 2 + topic_names_[i].length();
	}

	return size;
}

void MetadataRequest::Print()
{
	std::cout << "-----MetadataRequest-----" << std::endl;
	Request::Print();
	for (unsigned int i = 0; i < topic_names_.size(); i++)
	{
		std::cout << "topic name = " << topic_names_[i] << std::endl;
	}
	std::cout << "-------------------------" << std::endl;
}
//------------------------------SyncGroupRequest
#if 0
MemberAssignment::MemberAssignment(const std::vector<std::string> &topics)
{
	version_ = 0;
	subscription_ = topics;
	// user_data_ is empty 
}

GroupAssignment::GroupAssignment(const std::vector<std::string> &topics, const std::string &member_id)
	: member_assignment_(topics)
{
	member_id_ = member_id;
}

SyncGroupRequest::SyncGroupRequest(int correlation_id, const std::string &group_id, int generation_id,
		const std::string &member_id, )
{

}
#endif
