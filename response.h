#ifndef _RESPONSE_H_
#define _RESPONSE_H_

#include <string>

//------------------------------Head
class Response {
public:
	Response(short api_key, int correlation_id);
	virtual ~Response() {}

	short api_key_;			// it's not a part of protocol 
	
	int total_size_;		// exclude itself
	int correlation_id_;
};


//------------------------------GroupCoordinatorResponse
class GroupCoordinatorResponse: public Response {
public:
	GroupCoordinatorResponse(short api_key, int correlation_id, short error_code,
		int coordinator_id, const std::string &coordinator_host, int coordinator_port);

	short error_code_;
	int coordinator_id_;
	std::string coordinator_host_;
	int coordinator_port_;
};


//------------------------------JoinGroupResponse
class Member {
public:
	std::string member_id_;
	int member_metadata_size_;
	char *member_metadata_;
};

#if 0
class JoinGroupResponse: public Response {
public:
	JoinGroupResponse::JoinGroupResponse(int correlation_id, short error_code, int generation_id,
		std::string group_protocol, std::string leader_id, std::string member_id,
		std::vector<Member> members);

	short error_code_;
	int generation_id_;
	std::string group_protocol_;
	std::string leader_id_;
	std::string member_id_;
	std::vector<Member> members_;
};
#endif

#endif
