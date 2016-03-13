#ifndef _RESPONSE_H_
#define _RESPONSE_H_

#include <string>
#include <vector>
#include <map>

#include "member_assignment.h"
#include "request_response_type.h"

//------------------------------Head
class Response {
public:
	Response(short api_key, int correlation_id);
	Response(short api_key, char **buf);
	virtual ~Response() {}

	virtual int CountSize();
	virtual void PrintAll();

	short api_key_;			// it's not a part of protocol 
	int total_size_;		// exclude itself
	int correlation_id_;
};

//------------------------------GroupCoordinatorResponse
class GroupCoordinatorResponse: public Response {
public:
	GroupCoordinatorResponse(int correlation_id, short error_code,
		int coordinator_id, const std::string &coordinator_host, int coordinator_port);

	GroupCoordinatorResponse(char **buf);

	virtual int CountSize();
	virtual void PrintAll();

	short error_code_;
	int coordinator_id_;
	std::string coordinator_host_;
	int coordinator_port_;
};


//------------------------------JoinGroupResponse
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
	std::vector<std::string> GetAllMembers();

	virtual int CountSize();
	virtual void PrintAll();

	int GetGenerationId();
	std::string GetMemberId();

	short error_code_;
	int generation_id_;
	std::string group_protocol_;
	std::string leader_id_;
	std::string member_id_;
	std::vector<Member> members_;	// array
};


#endif

