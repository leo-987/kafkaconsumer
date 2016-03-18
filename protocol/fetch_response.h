#ifndef _FETCH_RESPONSE_H_
#define _FETCH_RESPONSE_H_

#include <vector>
#include <string>

#include "response.h"
#include "message_set.h"

class PartitionResponseInfo {
public:
	PartitionResponseInfo(char **buf);

	int CountSize();
	void PrintAll();

private:
	int32_t partition_;
	int16_t error_code_;
	int64_t high_water_mark_offset_;
	int32_t message_set_size_;	// The size in bytes of the message set for this partition
	MessageSet message_set_;
};

class TopicPartitionResponseInfo {
public:
	TopicPartitionResponseInfo(char **buf);

	int CountSize();
	void PrintAll();

private:
	std::string topic_name_;
	std::vector<PartitionResponseInfo> partitions_info_;
};

class FetchResponse: public Response {
public:
	FetchResponse(char **buf);
	virtual ~FetchResponse() {}

	virtual int CountSize();
	virtual void PrintAll();
	
private:
	std::vector<TopicPartitionResponseInfo> topic_partitions_;
	int32_t throttle_time_;
};

#endif
