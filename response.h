#ifndef _RESPONSE_H_
#define _RESPONSE_H_

#include <string>
#include <vector>

//------------------------------Head
class Response {
public:
	Response(short api_key, int correlation_id);
	virtual ~Response() {}

	virtual int Size();
	virtual void Print();

	short api_key_;			// it's not a part of protocol 
	int total_size_;		// exclude itself
	int correlation_id_;
};


//------------------------------GroupCoordinatorResponse
class GroupCoordinatorResponse: public Response {
public:
	GroupCoordinatorResponse(short api_key, int correlation_id, short error_code,
		int coordinator_id, const std::string &coordinator_host, int coordinator_port);

	virtual int Size();
	virtual void Print();

	short error_code_;
	int coordinator_id_;
	std::string coordinator_host_;
	int coordinator_port_;
};


//------------------------------JoinGroupResponse
class Member {
public:
	Member(const std::string &member_id, const std::string &member_metadata);

	int Size();

	std::string member_id_;
	std::string member_metadata_;	// bytes
};

class JoinGroupResponse: public Response {
public:
	JoinGroupResponse(short api_key, int correlation_id, short error_code,
			int generation_id, const std::string &group_protocol, const std::string &leader_id,
			const std::string &member_id, const std::vector<Member> &members);

	virtual int Size();
	virtual void Print();

	short error_code_;
	int generation_id_;
	std::string group_protocol_;
	std::string leader_id_;
	std::string member_id_;
	std::vector<Member> members_;
};

#endif
