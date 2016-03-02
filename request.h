#ifndef _REQUEST_H_
#define _REQUEST_H_

#include <string>
#include <vector>
#include <map>
#include <utility>

#include "member_assignment.h"
#include "request_response_type.h"

//------------------------------Head
class Request {
public:
	Request(short api_key, int correlation_id, std::string client_id = "consumer-1");
	virtual ~Request() {}

	virtual int CountSize();
	virtual void PrintAll();
	virtual int Package(char **buf);

	int   total_size_;		// exclude itself
	short api_key_;
	short api_version_;		// always 0
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

//------------------------------MetadataRequest
class MetadataRequest: public Request {
public:
	MetadataRequest(int correlation_id, const std::vector<std::string> &topic_names,
			bool for_all_topic = false);

	virtual int CountSize();
	virtual void PrintAll();
	virtual int Package(char **buf);

	std::vector<std::string> topic_names_;
};

//------------------------------SyncGroupRequest
class GroupAssignment {
public:
	GroupAssignment(const std::string &topic, const std::string &member_id,
		const std::vector<int> &partitions);

	int CountSize();
	void PrintAll();
	int Package(char **buf);

	std::string member_id_;
	MemberAssignment member_assignment_;	// bytes
};

class SyncGroupRequest: public Request {
public:
	SyncGroupRequest(int correlation_id, const std::string &topic, const std::string group_id,
			int generation_id, const std::string &member_id,
			const std::map<std::string, std::vector<int>> &member_partition_map);

	virtual int CountSize();
	virtual void PrintAll();
	virtual int Package(char **buf);

	std::string group_id_;
	int generation_id_;
	std::string member_id_;
	std::vector<GroupAssignment> group_assignment_;		// array
};

//------------------------------HeartbeatRequest
class HeartbeatRequest: public Request {
public:
	HeartbeatRequest(int correlation_id, const std::string &group_id, int generation_id,
					 const std::string &member_id);

	virtual int CountSize();
	virtual void PrintAll();
	virtual int Package(char **buf);

	std::string group_id_;
	int generation_id_;
	std::string member_id_;
};

#endif



