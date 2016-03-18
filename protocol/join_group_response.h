#ifndef _JOIN_GROUP_RESPONSE_H_
#define _JOIN_GROUP_RESPONSE_H_

#include <vector>
#include <string>
#include "response.h"

class Member {
public:
	Member(const std::string &member_id, const std::string &member_metadata);
	Member(char **buf);

	int CountSize();
	void PrintAll();

	std::string member_id_;
	std::string member_metadata_;	// bytes
};

class JoinGroupResponse: public Response {
public:
	JoinGroupResponse(int correlation_id, short error_code,
			int generation_id, const std::string &group_protocol, const std::string &leader_id,
			const std::string &member_id, const std::vector<Member> &members);

	JoinGroupResponse(char **buf);
	virtual ~JoinGroupResponse() {}


	virtual int CountSize();
	virtual void PrintAll();

	int GetGenerationId();
	std::string GetMemberId();
	bool IsGroupLeader();
	std::vector<std::string> GetAllMembers();

	short error_code_;
	int generation_id_;
	std::string group_protocol_;
	std::string leader_id_;
	std::string member_id_;
	std::vector<Member> members_;	// array
};
#endif
