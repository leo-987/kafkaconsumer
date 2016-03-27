#ifndef _OFFSET_COMMIT_RESPONSE_H_
#define _OFFSET_COMMIT_RESPONSE_H_

#include <vector>
#include <string>

#include "response.h"

class PartitionE {
public:
	PartitionE(char **buf);

	int CountSize();
	void PrintAll();
	int16_t GetErrorCode();
	
private:
	int32_t partition_;
	int16_t error_code_;
};

class TopicPartitionE {
public:
	TopicPartitionE(char **buf);

	int CountSize();
	void PrintAll();

	friend class OffsetCommitResponse;
private:
	std::string topic_;
	std::vector<PartitionE> partitions_;
};

// format: [TopicName [Partition ErrorCode]]
class OffsetCommitResponse: public Response {
public:
	OffsetCommitResponse(char **buf);

	virtual int CountSize();
	virtual void PrintAll();
	int16_t GetErrorCode();

private:
	std::vector<TopicPartitionE> topic_partitions_;
};

#endif
