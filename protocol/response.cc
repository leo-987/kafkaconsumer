#include <iostream>
#include <string.h>

#include "response.h"
//#include "request.h"
#include "util.h"

//------------------------------Head
Response::Response(short api_key, int correlation_id)
{
	api_key_ = api_key;
	total_size_ = 0;
	correlation_id_ = correlation_id;
}

Response::Response(short api_key, char **buf)
{
	api_key_ = api_key;
	total_size_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	correlation_id_ = Util::NetBytesToInt(*buf); 
	(*buf) += 4;
}

int Response::CountSize()
{
	return 4;
}

void Response::PrintAll()
{
	std::cout << "api key = " << api_key_ << std::endl;
	std::cout << "total size = " << total_size_ << std::endl;
	std::cout << "correlation id = " << correlation_id_ << std::endl;
}

//------------------------------GroupCoordinatorResponse
GroupCoordinatorResponse::GroupCoordinatorResponse(int correlation_id, short error_code,
		int coordinator_id, const std::string &coordinator_host, int coordinator_port)
	: Response(ApiKey::GroupCoordinatorType, correlation_id)
{
	error_code_ = error_code;
	coordinator_id_ = coordinator_id;
	coordinator_host_ = coordinator_host;
	coordinator_port_ = coordinator_port;
	total_size_ = CountSize();
}

GroupCoordinatorResponse::GroupCoordinatorResponse(char **buf)
	: Response(ApiKey::GroupCoordinatorType, buf)
{
	error_code_ = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	coordinator_id_ = Util::NetBytesToInt(*buf); 
	(*buf) += 4;
	short host_size = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	coordinator_host_ = std::string(*buf, host_size);
	(*buf) += host_size;
	coordinator_port_ = Util::NetBytesToInt(*buf); 

	if (total_size_ != CountSize())
	{
		throw "total size != count size are not equal";
	}
}

int GroupCoordinatorResponse::CountSize()
{
	int size = 0;
	size = Response::CountSize() + 2 /* error_code */ + 4 /* coordinator_id */+
		   2 /* coordinator_host size */ + coordinator_host_.length() + 4;
	return size;
}

void GroupCoordinatorResponse::PrintAll()
{
	std::cout << "-----GroupCoordinatorResponse-----" << std::endl;
	Response::PrintAll();
	std::cout << "error code = " << error_code_ << std::endl;
	std::cout << "coordinator id = " << coordinator_id_ << std::endl;
	std::cout << "coordinator host = " << coordinator_host_ << std::endl;
	std::cout << "coordinator port = " << coordinator_port_ << std::endl;
	std::cout << "----------------------------------" << std::endl;
}

//------------------------------JoinGroupResponse
Member::Member(const std::string &member_id, const std::string &member_metadata)
{
	member_id_ = member_id;
	member_metadata_ = member_metadata;
}

Member::Member(char **buf)
{
	// MemberId
	short member_id_size = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	member_id_ = std::string(*buf, member_id_size);
	(*buf) += member_id_size;

	// MemberMetadata
	int member_metadata_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	member_metadata_ = std::string(*buf, member_metadata_size);
	(*buf) += member_metadata_size;
}

int Member::CountSize()
{
	return 2 + member_id_.length() +
		   4 /* bytes */ + member_metadata_.length();
}

void Member::PrintAll()
{
	std::cout << "member id = " << member_id_ << std::endl;
	std::cout << "member metadata = " << member_metadata_ << std::endl;
}

JoinGroupResponse::JoinGroupResponse(int correlation_id, short error_code,
		int generation_id, const std::string &group_protocol, const std::string &leader_id,
		const std::string &member_id, const std::vector<Member> &members)
	: Response(ApiKey::JoinGroupType, correlation_id)
{
	error_code_ = error_code;
	generation_id_ = generation_id;
	group_protocol_ = group_protocol;
	leader_id_ = leader_id;
	member_id_ = member_id;
	members_ = members;

	total_size_ = CountSize();
}

JoinGroupResponse::JoinGroupResponse(char **buf)
	: Response(ApiKey::JoinGroupType, buf)
{
	error_code_ = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	generation_id_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;

	// GroupProtocol
	short group_protocol_size = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	group_protocol_ = std::string(*buf, group_protocol_size);
	(*buf) += group_protocol_size;

	// LeaderId
	short leader_id_size = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	leader_id_ = std::string(*buf, leader_id_size);
	(*buf) += leader_id_size;

	// MemberId
	short member_id_size = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	member_id_ = std::string(*buf, member_id_size);
	(*buf) += member_id_size;

	// Members
	int members_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int i = 0; i < members_size; i++)
	{
		Member member(buf);
		members_.push_back(member);
	}

	if (total_size_ != CountSize())
	{
		throw "CountSize are not equal";
	}
}

int JoinGroupResponse::CountSize()
{
	int size = 0;

	size = Response::CountSize() + 2 /* error_code */ + 4 /* generation_id */+
		   2 /* group_protocol size */ + group_protocol_.length() +
		   2 /* leader_id size */ + leader_id_.length() +
		   2 /* member_id size */ + member_id_.length();

	// array
	size += 4;
	for (unsigned int i = 0; i < members_.size(); i++)
		size += members_[i].CountSize();

	return size;
}

void JoinGroupResponse::PrintAll()
{
	std::cout << "-----JoinGroupResponse-----" << std::endl;
	Response::PrintAll();
	std::cout << "error code = " << error_code_ << std::endl;
	std::cout << "generation id = " << generation_id_ << std::endl;
	std::cout << "group protocol = " << group_protocol_ << std::endl;
	std::cout << "leader id = " << leader_id_ << std::endl;
	std::cout << "member id = " << member_id_ << std::endl;
	std::cout << "members:" << std::endl;
	for (auto it = members_.begin(); it != members_.end(); ++it)
	{
		it->PrintAll();
	}
	std::cout << "---------------------------" << std::endl;
}

std::vector<std::string> JoinGroupResponse::GetAllMembers()
{
	std::vector<std::string> members;

	for (auto it = members_.begin(); it != members_.end(); ++it)
		members.push_back(it->member_id_);

	return members;
}

int JoinGroupResponse::GetGenerationId()
{
	return generation_id_;
}

std::string JoinGroupResponse::GetMemberId()
{
	return member_id_;
}
