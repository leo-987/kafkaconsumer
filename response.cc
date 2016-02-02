#include <iostream>

#include "response.h"


//------------------------------Head
Response::Response(short api_key, int correlation_id)
{
	api_key_ = api_key;
	total_size_ = 0;
	correlation_id_ = correlation_id;
}

void Response::Print()
{
	std::cout << "api key = " << api_key_ << std::endl;
	std::cout << "total size = " << total_size_ << std::endl;
	std::cout << "correlation id = " << correlation_id_ << std::endl;
}

//------------------------------GroupCoordinatorResponse
GroupCoordinatorResponse::GroupCoordinatorResponse(short api_key, int correlation_id, short error_code,
		int coordinator_id, const std::string &coordinator_host, int coordinator_port)
	: Response(api_key, correlation_id)
{
	error_code_ = error_code;
	coordinator_id_ = coordinator_id;
	coordinator_host_ = coordinator_host;
	coordinator_port_ = coordinator_port;
	total_size_ = 4 /* correlation_id */ + 2 /* error_code */ + 4 /* coordinator_id */+
				  2 /* coordinator_host size */ + coordinator_host_.length() + 4;
}

void GroupCoordinatorResponse::Print()
{
	std::cout << "-----GroupCoordinatorResponse-----" << std::endl;
	Response::Print();
	std::cout << "error code = " << error_code_ << std::endl;
	std::cout << "coordinator id = " << coordinator_id_ << std::endl;
	std::cout << "coordinator host = " << coordinator_host_ << std::endl;
	std::cout << "coordinator port = " << coordinator_port_ << std::endl;
}

//------------------------------JoinGroupResponse
Member::Member(const std::string &member_id, const std::string &member_metadata)
{
	member_id_ = member_id;
	member_metadata_ = member_metadata;
}

JoinGroupResponse::JoinGroupResponse(short api_key, int correlation_id, short error_code,
		int generation_id, const std::string &group_protocol, const std::string &leader_id,
		const std::string &member_id, const std::vector<Member> &members)
	: Response(api_key, correlation_id)
{
	error_code_ = error_code;
	generation_id_ = generation_id;
	group_protocol_ = group_protocol;
	leader_id_ = leader_id;
	member_id_ = member_id;
	members_ = members;

	int members_size = 4;
	for (unsigned int i = 0; i < members_.size(); i++)
	{
		members_size += 2 + members_[i].member_id_.length() +
						4 + members_[i].member_metadata_.length();
	}
	total_size_ = 4 /* correlation_id */ + 2 /* error_code */ + 4 /* generation_id */+
				  2 /* group_protocol size */ + group_protocol_.length() +
				  2 /* leader_id size */ + leader_id_.length() +
				  2 /* member_id size */ + member_id_.length() +
				  members_size;
}

void JoinGroupResponse::Print()
{
	std::cout << "-----JoinGroupResponse-----" << std::endl;
	Response::Print();
	std::cout << "error code = " << error_code_ << std::endl;
	std::cout << "generation id = " << generation_id_ << std::endl;
	std::cout << "group protocol = " << group_protocol_ << std::endl;
	std::cout << "leader id = " << leader_id_ << std::endl;
	std::cout << "member id = " << member_id_ << std::endl;
	std::cout << "members:" << std::endl;
	for (unsigned int i = 0; i < members_.size(); i++)
	{
		Member &member = members_[i];
		std::cout << "	member id = " << member.member_id_ << std::endl;
		std::cout << "	member metadata = " << member.member_metadata_ << std::endl;
	}
}
