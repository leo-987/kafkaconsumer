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
	
private:
	int32_t partition_;
	int16_t error_code_;
};

class TopicPartitionE {
public:
	TopicPartitionE(char **buf);

	int CountSize();
	void PrintAll();

private:
	std::string topic_;
	std::vector<PartitionE> partitions_;
};

class OffsetCommitResponse: public Response {
public:
	OffsetCommitResponse(char **buf);

	virtual int CountSize();
	virtual void PrintAll();

private:
	std::vector<TopicPartitionE> topic_partitions_;
};

#endif
