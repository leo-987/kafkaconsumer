#ifndef _REQUEST_H_
#define _REQUEST_H_

#include <string>
#include <vector>
#include <map>
#include <utility>

#include "member_assignment.h"

//------------------------------Head
class Request {
public:
	Request(short api_key, int correlation_id, short api_version = 0, const std::string client_id = "consumer-1");
	virtual ~Request() {}

	virtual int CountSize();
	virtual void PrintAll();
	virtual int Package(char **buf);

	int GetCorrelationId();
	short GetApiKey();

	int   total_size_;		// exclude itself
	short api_key_;
	short api_version_;
	int   correlation_id_;
	std::string client_id_;
};

//------------------------------GroupCoordinatorRequest
class GroupCoordinatorRequest: public Request {
public:
	GroupCoordinatorRequest(int correlation_id, const std::string &group_id);

	virtual int CountSize();
	virtual void PrintAll();
	virtual int Package(char **buf);

	std::string group_id_;
};

//------------------------------JoinGroupRequest
class ProtocolMetadata {
public:
	ProtocolMetadata(const std::vector<std::string> &topics);

	int CountSize();
	int Package(char **buf);

	short version_;
	std::vector<std::string> subscription_;
	std::string user_data_;		// bytes
};

class GroupProtocol {
public:
	GroupProtocol(const std::vector<std::string> &topics);

	int CountSize();
	int Package(char **buf);

	std::string assignment_strategy_;		// ProtocolName = range
	ProtocolMetadata protocol_metadata_;	// byts
};

class JoinGroupRequest: public Request {
public:
	JoinGroupRequest(int correlation_id,
		const std::string &group_id, const std::string member_id,
		const std::vector<std::string> &topics);

	virtual int CountSize();
	virtual void PrintAll();
	virtual int Package(char **buf);

	std::string group_id_;
	int session_timeout_;
	std::string member_id_;			// When a member first joins the group, the memberId will be empty
	std::string protocol_type_;		// "consumer"
	std::vector<GroupProtocol> group_protocols_;
};
#endif

