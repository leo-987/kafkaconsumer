#ifndef _FETCH_RESPONSE_H_
#define _FETCH_RESPONSE_H_

#include <vector>
#include <string>

#include "response.h"
#include "message_set.h"

class PartitionInfo {
public:
	PartitionInfo(char **buf);

	int CountSize();
	void PrintAll();

	friend class TopicPartitionInfo;
	friend class FetchResponse;

private:
	int32_t partition_;
	int16_t error_code_;
	int64_t high_water_mark_offset_;
	int32_t message_set_size_;	// The size in bytes of the message set for this partition
	MessageSet message_set_;
};

class TopicPartitionInfo {
public:
	TopicPartitionInfo(char **buf);

	int CountSize();
	void PrintAll();

	friend class FetchResponse;

private:
	std::string topic_;
	std::vector<PartitionInfo> partitions_info_;
};

class FetchResponse: public Response {
public:
	FetchResponse(char **buf);
	virtual ~FetchResponse() {}

	virtual int CountSize();
	virtual void PrintAll();

	void PrintTopicAndMsg();
	bool IsEmptyMsg();
	int64_t GetLastOffset();
	
private:
	std::vector<TopicPartitionInfo> topic_partitions_;
	int32_t throttle_time_;
};

#endif
