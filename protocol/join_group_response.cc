#include "join_group_response.h"
#include "util.h"
#include "easylogging++.h"

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
	LOG(DEBUG) << "member id = " << member_id_;
	LOG(DEBUG) << "member metadata = " << member_metadata_;
}

//---------------------------------------------------
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
		std::shared_ptr<Member> member = std::make_shared<Member>(buf);
		members_.push_back(member);
	}

	if (Response::GetTotalSize() != CountSize())
	{
		LOG(ERROR) << "CountSize are not equal";
		throw;
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
	for (auto m_it = members_.begin(); m_it != members_.end(); ++m_it)
		size += (*m_it)->CountSize();

	return size;
}

void JoinGroupResponse::PrintAll()
{
	LOG(DEBUG) << "-----JoinGroupResponse-----";
	Response::PrintAll();
	LOG(DEBUG) << "error code = " << error_code_;
	LOG(DEBUG) << "generation id = " << generation_id_;
	LOG(DEBUG) << "group protocol = " << group_protocol_;
	LOG(DEBUG) << "leader id = " << leader_id_;
	LOG(DEBUG) << "member id = " << member_id_;
	LOG(DEBUG) << "members:";
	for (auto it = members_.begin(); it != members_.end(); ++it)
		(*it)->PrintAll();
	LOG(DEBUG) << "---------------------------";
}

bool JoinGroupResponse::IsGroupLeader()
{
	return !members_.empty();
}

std::vector<std::string> JoinGroupResponse::GetAllMembers()
{
	std::vector<std::string> members;

	for (auto it = members_.begin(); it != members_.end(); ++it)
		members.push_back((*it)->member_id_);

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
