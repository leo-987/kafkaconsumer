#ifndef _OFFSET_FETCH_RESPONSE_H_
#define _OFFSET_FETCH_RESPONSE_H_

#include <string>

#include "response.h"
#include "util.h"

class PartitionOffsetInfo {
public:
	PartitionOffsetInfo(char **buf);

	int CountSize();
	void PrintAll();

	int partition_;
	long offset_;
	std::string metadata_;
	short error_code_;
};

class OffsetFetchResponse: public Response {
public:
	OffsetFetchResponse(char **buf);

	virtual ~OffsetFetchResponse() {}

	virtual int CountSize();
	virtual void PrintAll();

	int ParseOffset(std::map<int, long> &partition_offset);
	
private:
	// XXX: [TopicName [Partition Offset Metadata ErrorCode]]
	std::string topic_;
	std::vector<PartitionOffsetInfo> partitions_info_;
};

#endif
