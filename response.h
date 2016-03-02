#ifndef _RESPONSE_H_
#define _RESPONSE_H_

#include <string>
#include <vector>

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

//------------------------------MetadataResponse
class Broker {
public:
	Broker(int node_id, const std::string &host, int port);
	Broker(char **buf);

	int CountSize();
	void PrintAll();

	int node_id_;
	std::string host_;
	int port_;
};

class PartitionMetadata {
public:
	PartitionMetadata(short error_code, int partition_id, int leader,
		const std::vector<int> &replicas, const std::vector<int> &irs);

	PartitionMetadata(char **buf);

	int CountSize();
	void PrintAll();

	short partition_error_code_;
	int partition_id_;
	int leader_;
	std::vector<int> replicas_;		// array
	std::vector<int> isr_;			// array
};

class TopicMetadata {
public:
	TopicMetadata(short error_code, const std::string &topic_name,
		const std::vector<PartitionMetadata> &partition_metadata);

	TopicMetadata(char **buf);

	int CountSize();
	void PrintAll();

	short topic_error_code_;
	std::string topic_name_;
	std::vector<PartitionMetadata> partition_metadata_;
};

class MetadataResponse: public Response {
public:
	MetadataResponse(int correlation_id, const std::vector<Broker> &brokers,
		const std::vector<TopicMetadata> &topic_metadata);

	MetadataResponse(char **buf);

	virtual int CountSize();
	virtual void PrintAll();

	int GetBrokerIdFromHostname(const std::string &hostname);
		
	std::vector<Broker> brokers_;				// array
	std::vector<TopicMetadata> topic_metadata_;	// array
};

//------------------------------SyncGroupResponse
class SyncGroupResponse: public Response {
public:
	SyncGroupResponse(char **buf);

	virtual int CountSize();
	virtual void PrintAll();

	short error_code_;
	MemberAssignment member_assignment_;	// bytes
};

//------------------------------HeartbeatResponse
class HeartbeatResponse: public Response {
public:
	HeartbeatResponse(char **buf);

	virtual int CountSize();
	virtual void PrintAll();

	short error_code_;
};

#endif

