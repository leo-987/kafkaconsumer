#ifndef _METADATA_RESPONSE_H_
#define _METADATA_RESPONSE_H_

#include <string>
#include <vector>
#include <map>

#include "response.h"
#include "broker.h"
#include "partition.h"

#if 0
class Broker {
public:
	Broker(int fd, int node_id, const std::string &host, int port);
	Broker(char **buf);

	int CountSize();
	void PrintAll();

	int node_id_;
	std::string host_;
	int port_;
};
#endif

class PartitionMetadata {
public:
	PartitionMetadata(short error_code, int partition_id, int leader,
		const std::vector<int> &replicas, const std::vector<int> &irs);

	PartitionMetadata(char **buf);

	int CountSize();
	void PrintAll();

	int16_t partition_error_code_;
	int32_t partition_id_;
	int32_t leader_;
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

	int16_t topic_error_code_;
	std::string topic_;
	std::vector<PartitionMetadata> partition_metadata_;
};

// [Broker][TopicMetadata]
class MetadataResponse: public Response {
public:
	MetadataResponse(char **buf);
	virtual ~MetadataResponse() {}

	virtual int CountSize();
	virtual void PrintAll();

	int GetBrokerIdFromHostname(const std::string &hostname);
	void ParseBrokers(std::map<int, Broker> &brokers);
	void ParsePartitions(std::map<int, Partition> &partitions);

	std::vector<Broker> brokers_;				// array
	std::vector<TopicMetadata> topic_metadata_;	// array
};

#endif
