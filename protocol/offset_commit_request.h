#ifndef _OFFSET_COMMIT_REQUEST_H_
#define _OFFSET_COMMIT_REQUEST_H_

#include <string>
#include <vector>

#include "request.h"
#include "request_response_type.h"

class PartitionOM {
public:
	PartitionOM(int32_t partition, int64_t offset, std::string metadata = "");
	int CountSize();
	void PrintAll();
	void Package(char **buf);

private:
	int32_t partition_;
	int64_t offset_;
	std::string metadata_;
};

class TopicPartitionOM {
public:
	TopicPartitionOM(const std::string &topic, int32_t partition, int64_t offset);
	TopicPartitionOM(const std::string &topic, const std::vector<PartitionOM> &partitions);
	int CountSize();
	void PrintAll();
	void Package(char **buf);

private:
	std::string topic_;

	// [Partition Offset Metadata]
	std::vector<PartitionOM> partitions_;
};

// format: ConsumerGroup ConsumerGroupGenerationId ConsumerId RetentionTime [TopicName [Partition Offset Metadata]]
class OffsetCommitRequest: public Request {
public:
	OffsetCommitRequest(const std::string &group, int32_t group_generation_id, const std::string &consumer_id, 
		const std::string &topic, int32_t partition, int64_t offset, int correlation_id = ApiKey::OffsetCommitType);
	OffsetCommitRequest(const std::string &group, int32_t group_generation_id, const std::string &consumer_id, 
		const std::string &topic, const std::vector<PartitionOM> &partitions, int correlation_id = ApiKey::OffsetCommitType);
	virtual int CountSize();
	virtual void PrintAll();
	virtual void Package(char **buf);

private:
	std::string group_;
	int32_t group_generation_id_;
	std::string consumer_id_;
	int64_t retention_time_;

	// [TopicName [Partition Offset Metadata]]
	std::vector<TopicPartitionOM> topic_partitions_;
};

#endif
