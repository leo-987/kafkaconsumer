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

	int partition_;
	short error_code_;
	long high_water_mark_offset_;
	int message_set_size_;	// The size in bytes of the message set for this partition
	MessageSet message_set_;
};

class TopicPartitionResponseInfo {
public:
	TopicPartitionResponseInfo(char **buf);

	int CountSize();
	void PrintAll();

	std::string topic_name_;
	std::vector<PartitionResponseInfo> partitions_info_;

};

class FetchResponse: public Response {
public:
	FetchResponse(char **buf);

	virtual int CountSize();
	virtual void PrintAll();
	
	std::vector<TopicPartitionResponseInfo> topic_partitions_;
	int throttle_time_;
};

#endif
