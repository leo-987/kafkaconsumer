#include <iostream>

#include "request.h"

//------------------------------Head
Request::Request(short api_key, int correlation_id, std::string client_id)
{
	total_size_ = 0;
	api_key_ = api_key;
	api_version_ = 0;
	correlation_id_ = correlation_id;
	client_id_size_ = client_id.length();
	client_id_ = client_id;
}


//------------------------------GroupCoordinatorRequest
GroupCoordinatorRequest::GroupCoordinatorRequest(int correlation_id, const std::string &group_id)
	: Request(10, correlation_id)
{
	group_id_size_ = group_id.length();
	group_id_ = group_id;
	total_size_ = 2 + 2 + 4 + 2 + client_id_size_ + 2 + group_id_size_;
}


//------------------------------JoinGroupRequest
ProtocolMetadata::ProtocolMetadata(const std::vector<std::string> &topics)
{
	version_ = 0;
	topics_size_ = topics.size();
	for (int i = 0; i < topics_size_; i++)
	{
		topics_.push_back({topics[i].length(), topics[i]});
	}
	user_data_size_ = 0;
	user_data_ = NULL;
}


GroupProtocols::GroupProtocols(const std::vector<std::string> &topics)
	: protocol_metadata_(topics)
{
	assignment_strategy_size_ = 5;
	assignment_strategy_ = "range";
}


JoinGroupRequest::JoinGroupRequest(int correlation_id,
		const std::string &group_id, const std::string member_id,
		const std::vector<std::string> &topics)
	: Request(11, correlation_id), group_protocol_(topics)
{
	group_id_size_ = group_id.length();
	group_id_ = group_id;
	session_timeout_ = 30000;
	member_id_size_ = member_id.length();
	member_id_ = member_id;
	protocol_type_size_ = 8;
	protocol_type_ = "consumer";
	group_protocol_size_ = 1;

	int tmp_len = 0;
	for (int i = 0; i < topics.size(); i++)
		tmp_len += 2 + topics[i].length();

	total_size_ = 2 + 2 + 4 + 2 + client_id_size_ + 2 + group_id_size_ +
		4 + 2 + member_id_.length() + 2 + protocol_type_size_ +
		2 + 4 + group_protocol_.assignment_strategy_size_ + 2 + 4 + tmp_len + 4 +
		group_protocol_.protocol_metadata_.user_data_size_;
}
