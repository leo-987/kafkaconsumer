#ifndef _JOIN_GROUP_REQUEST_H_
#define _JOIN_GROUP_REQUEST_H_

#include <vector>
#include <string>

#include "request.h"

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
