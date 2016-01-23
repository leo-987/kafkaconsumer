#ifndef _REQUEST_H_
#define _REQUEST_H_

#include <string>
#include <vector>
#include <utility>


//------------------------------Head
class Request {
public:
	Request(short api_key, int correlation_id, std::string client_id = "client_oyld");

	
	int   total_size_;		// exclude itself;
	short api_key_;
	short api_version_;		// always 0
	int   correlation_id_;
	std::string client_id_;
};


//------------------------------GroupCoordinatorRequest
class GroupCoordinatorRequest: public Request {
public:
	GroupCoordinatorRequest(int correlation_id, const std::string &group_id);

	std::string group_id_;
};


//------------------------------JoinGroupRequest
class ProtocolMetadata {
public:
	ProtocolMetadata(const std::vector<std::string> &topics);

	short version_;
	std::vector<std::string> subscription_;
	std::vector<char> user_data_;
};


class GroupProtocol {
public:
	GroupProtocol(const std::vector<std::string> &topics);

	std::string assignment_strategy_;	// ProtocolName = range
	ProtocolMetadata protocol_metadata_;
};


class JoinGroupRequest: public Request {
public:
	JoinGroupRequest(int correlation_id,
		const std::string &group_id, const std::string member_id,
		const std::vector<std::string> &topics);

	std::string group_id_;
	int session_timeout_;
	std::string member_id_;
	std::string protocol_type_;		// "consumer"
	std::vector<GroupProtocol> group_protocols_;
};

#endif
