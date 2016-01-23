#ifndef _REQUEST_H_
#define _REQUEST_H_

#include <string>
#include <vector>
#include <utility>


//------------------------------Head
class Request {
public:
	Request(short api_key, int correlation_id, std::string client_id = "client_oyld");

	// exclude itself
	int   total_size_;
	short api_key_;
	short api_version_;
	int   correlation_id_;
	short client_id_size_;
	std::string client_id_;
};


//------------------------------GroupCoordinatorRequest
class GroupCoordinatorRequest: public Request {
public:
	GroupCoordinatorRequest(int correlation_id, const std::string &group_id);

	short group_id_size_;
	std::string group_id_;
};


//------------------------------JoinGroupRequest
class ProtocolMetadata {
public:
	ProtocolMetadata(const std::vector<std::string> &topics);

	short version_;
	int topics_size_;
	std::vector<std::pair<short, std::string>> topics_;
	int user_data_size_;
	char *user_data_;
};


class GroupProtocols {
public:
	GroupProtocols(const std::vector<std::string> &topics);

	// ProtocolName
	short assignment_strategy_size_;
	std::string assignment_strategy_;
	ProtocolMetadata protocol_metadata_;
};


class JoinGroupRequest: public Request {
public:
	JoinGroupRequest(int correlation_id,
		const std::string &group_id, const std::string member_id,
		const std::vector<std::string> &topics);

	short group_id_size_;
	std::string group_id_;
	int session_timeout_;
	short member_id_size_;
	std::string member_id_;
	short protocol_type_size_;
	std::string protocol_type_;		// "consumer"
	int group_protocol_size_;
	GroupProtocols group_protocol_;
};

#endif
