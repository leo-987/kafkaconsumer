#ifndef _REQUEST_H_
#define _REQUEST_H_

#include <string>
#include <vector>
#include <utility>

// Api key is short, so use struct rather than enum
struct ApiKey {
	static const short ProduceRequest          = 0;
	static const short FetchRequest            = 1;
	static const short OffsetRequest           = 2;
	static const short MetadataRequest         = 3;
	static const short OffsetCommitRequest     = 8;
	static const short OffsetFetchRequest      = 9;
	static const short GroupCoordinatorRequest = 10;
	static const short JoinGroupRequest        = 11;
	static const short HeartbeatRequest        = 12;
	static const short LeaveGroupRequest       = 13;
	static const short SyncGroupRequest        = 14;
	static const short DescribeGroupsRequest   = 15;
	static const short ListGroupsRequest       = 16;
};

//------------------------------Head
class Request {
public:
	Request(short api_key, int correlation_id, std::string client_id = "consumer-1");
	virtual ~Request() {}

	virtual int Size();
	virtual void Print();
	
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

	virtual int Size();
	virtual void Print();

	std::string group_id_;
};

//------------------------------JoinGroupRequest
class ProtocolMetadata {
public:
	ProtocolMetadata(const std::vector<std::string> &topics);

	int Size();

	short version_;
	std::vector<std::string> subscription_;
	std::string user_data_;		// bytes
};

class GroupProtocol {
public:
	GroupProtocol(const std::vector<std::string> &topics);

	int Size();

	std::string assignment_strategy_;	// ProtocolName = range
	ProtocolMetadata protocol_metadata_;
};

class JoinGroupRequest: public Request {
public:
	JoinGroupRequest(int correlation_id,
		const std::string &group_id, const std::string member_id,
		const std::vector<std::string> &topics);

	virtual int Size();
	virtual void Print();

	std::string group_id_;
	int session_timeout_;
	std::string member_id_;			// When a member first joins the group, the memberId will be empty
	std::string protocol_type_;		// "consumer"
	std::vector<GroupProtocol> group_protocols_;
};

//------------------------------MetadataRequest
class MetadataRequest: public Request {
public:
	MetadataRequest(int correlation_id, const std::vector<std::string> &topic_names);

	virtual int Size();
	virtual void Print();

	std::vector<std::string> topic_names_;
};

//------------------------------SyncGroupRequest
#if 0
class MemberAssignment {
public:
	MemberAssignment();

	short version_;
	std::vector<PartitionAssignment> partition_assignment_;
	std::string user_data_;		// bytes
};

class GroupAssignment {
public:
	GroupAssignment();

	std::string member_id_;
	MemberAssignment member_assignment_;	// bytes
};

class SyncGroupRequest: public Request {
public:
	SyncGroupRequest();

	virtual void Print();

	std::string group_id_;
	int generation_id_;
	std::string member_id_;
	std::vector<GroupAssignment> group_assignment_;
};
#endif
#endif



