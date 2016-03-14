#ifndef _OFFSET_RESPONSE_
#define _OFFSET_RESPONSE_

#include <vector>

#include "response.h"

class PartitionOffsets {
public:
	PartitionOffsets(char **buf);

	int CountSize();
	void PrintAll();

	// Partition ErrorCode [Offset]
	int32_t partition_;
	int16_t error_code_;
	std::vector<int64_t> offset_array_;
};

class TopicPartitionOR {
public:
	TopicPartitionOR(char **buf);

	int CountSize();
	void PrintAll();

	// TopicName [PartitionOffsets]
	std::string topic_;
	std::vector<PartitionOffsets> partition_offset_array_;
};

class OffsetResponse: public Response {
public:
	OffsetResponse(char **buf);
	virtual ~OffsetResponse() {}

	virtual int CountSize();
	virtual void PrintAll();

	long GetNewOffset();

	// [TopicName [PartitionOffsets]]
	std::vector<TopicPartitionOR> topic_partition_array_;
};

#endif
