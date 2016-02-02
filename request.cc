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


//------------------------------GroupCoordinatorRequest
GroupCoordinatorRequest::GroupCoordinatorRequest(int correlation_id, const std::string &group_id)
	: Request(10, correlation_id)
{
	group_id_ = group_id;
	total_size_ = 2 + 2 + 4 + 2 + client_id_.length() +		// head
				  2 + group_id_.length();					// body
}


//------------------------------JoinGroupRequest
ProtocolMetadata::ProtocolMetadata(const std::vector<std::string> &topics)
{
	version_ = 0;
	subscription_ = topics;
	// user_data_ is empty 
}


GroupProtocol::GroupProtocol(const std::vector<std::string> &topics)
	: protocol_metadata_(topics)
{
	assignment_strategy_ = "range";
}


JoinGroupRequest::JoinGroupRequest(int correlation_id,
		const std::string &group_id, const std::string member_id,
		const std::vector<std::string> &topics)
	: Request(11, correlation_id)
{
	// only one group protocol
	GroupProtocol group_protocols(topics);
	group_protocols_.push_back(topics);

	group_id_ = group_id;
	session_timeout_ = 30000;
	member_id_ = member_id;
	protocol_type_ = "consumer";

	int array_len = 0;

	for (unsigned int i = 0; i < group_protocols_.size(); i++)
	{
		array_len += 2 + group_protocols_[i].assignment_strategy_.length() + 2 /* version */+ 4 /* array */;
		std::vector<std::string> &subscription = group_protocols_[i].protocol_metadata_.subscription_;
		for (unsigned int j = 0; j < subscription.size(); j++)
		{
			array_len += 2 + subscription[j].length();
		}
		array_len += 4 + group_protocols_[i].protocol_metadata_.user_data_.length();
	}

	total_size_ = 2 + 2 + 4 + 2 + client_id_.length() +		// head
				  2 + group_id_.length() + 4 + 2 + member_id_.length() + 2 + protocol_type_.length() +
				  4/* array */ + array_len;
}



