#ifndef _OFFSET_REQUEST_
#define _OFFSET_REQUEST_

#include <vector>
#include <string>

#include "request.h"

class PartitionTime {
public:
	PartitionTime(int32_t p, int64_t t, int32_t m = 1);

	int CountSize();
	void PrintAll();
	int Package(char **buf);

	// Partition Time MaxNumberOfOffsets
	int32_t partition_;
	int64_t time_;	// -1: latest -2: earliest
	int32_t max_number_of_offsets_;	// always 1
};

class TopicPartition {
public:
	TopicPartition(const std::string &topic, const std::vector<int32_t> &p, int64_t time);

	int CountSize();
	void PrintAll();
	int Package(char **buf);

	// TopicName [Partition Time MaxNumberOfOffsets]
	std::string topic_;
	std::vector<PartitionTime> partition_time_array_;
};

class OffsetRequest: public Request {
public:
	OffsetRequest(int correlation_id, const std::string &topic,
		const std::vector<int32_t> &p, int64_t time = -1, int32_t replica_id = -1);

	virtual ~OffsetRequest() {}
	virtual int CountSize();
	virtual void PrintAll();
	virtual int Package(char **buf);

	// ReplicaId [TopicName [Partition Time MaxNumberOfOffsets]]
	int32_t replica_id_;	// always -1
	std::vector<TopicPartition> topic_partition_array_;
};

#endif
