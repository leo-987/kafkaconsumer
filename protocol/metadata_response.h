#ifndef _METADATA_RESPONSE_H_
#define _METADATA_RESPONSE_H_

#include <string>
#include <vector>
#include <unordered_map>

#include "response.h"
#include "broker.h"
#include "partition.h"

class PartitionMetadata {
public:
	//PartitionMetadata(short error_code, int partition_id, int leader,
	//	const std::vector<int> &replicas, const std::vector<int> &irs);

	PartitionMetadata(char **buf);

	int CountSize();
	void PrintAll();

	friend class MetadataResponse;
private:
	int16_t partition_error_code_;
	int32_t partition_id_;
	int32_t leader_;
	std::vector<int> replicas_;		// array
	std::vector<int> isr_;			// array
};

class TopicMetadata {
public:
	//TopicMetadata(short error_code, const std::string &topic_name,
	//	const std::vector<PartitionMetadata> &partition_metadata);
	TopicMetadata(char **buf);

	int CountSize();
	void PrintAll();

	friend class MetadataResponse;
private:
	int16_t topic_error_code_;
	std::string topic_;
	std::vector<PartitionMetadata> partition_metadata_;
};

// format: [Broker][TopicMetadata]
class MetadataResponse: public Response {
public:
	MetadataResponse(char **buf);
	virtual ~MetadataResponse() {}

	virtual int CountSize();
	virtual void PrintAll();

	void ParseBrokers(std::unordered_map<int, Broker> &updated_brokers);
	int16_t ParsePartitions(std::unordered_map<int, Partition> &partitions);

private:
	std::vector<Broker> brokers_;				// array
	std::vector<TopicMetadata> topic_metadata_;	// array
	int GetFdFromIp(const std::string &alive_ip, const std::unordered_map<int, Broker> &origin_brokers);
};

#endif
