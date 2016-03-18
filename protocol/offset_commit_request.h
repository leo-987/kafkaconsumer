#ifndef _OFFSET_COMMIT_REQUEST_H_
#define _OFFSET_COMMIT_REQUEST_H_

#include <string>
#include "request.h"

// format: ConsumerGroup ConsumerGroupGenerationId ConsumerId RetentionTime [TopicName [Partition Offset Metadata]]
class OffsetCommitRequest: public Request {
public:
	OffsetCommitRequest(const std::string &group);

private:
	std::string consumer_group_id_;
	int32_t consumer_group_generation_id_;
	std::string consumer_id_;
	int64_t retention_time_;
	std::string topic_;
	int32_t partition_;
	int64_t offset_;
	std::string metadata_;
};

#endif
