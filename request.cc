#include <iostream>

#include "request.h"

short ApiKey::ProduceRequest          = 0;
short ApiKey::FetchRequest            = 1;
short ApiKey::OffsetRequest           = 2;
short ApiKey::MetadataRequest         = 3;
short ApiKey::OffsetCommitRequest     = 8;
short ApiKey::OffsetFetchRequest      = 9;
short ApiKey::GroupCoordinatorRequest = 10;
short ApiKey::JoinGroupRequest        = 11;
short ApiKey::HeartbeatRequest        = 12;
short ApiKey::LeaveGroupRequest       = 13;
short ApiKey::SyncGroupRequest        = 14;
short ApiKey::DescribeGroupsRequest   = 15;
short ApiKey::ListGroupsRequest       = 16;

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
	: Request(10, correlation_id)
{
	group_id_ = group_id;
	total_size_ = Request::Size() +	// head
				  Size();			// body
}

int GroupCoordinatorRequest::Size()
{
	return 2 + group_id_.length();
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
	: Request(11, correlation_id)
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
	int size = 0;
	size += Request::Size() +	// head
			2 + group_id_.length() + 4 + 2 + member_id_.length() + 2 + protocol_type_.length() +
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
