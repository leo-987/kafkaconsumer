#ifndef _OFFSET_FETCH_RESPONSE_H_
#define _OFFSET_FETCH_RESPONSE_H_

#include <string>
#include <memory>

#include "response.h"
#include "util.h"

class PartitionOffsetME {
public:
	PartitionOffsetME(char **buf);

	int CountSize();
	void PrintAll();
	int16_t GetErrorCode();
	int32_t GetPartition();
	int64_t GetOffset();

private:
	int32_t partition_;
	int64_t offset_;
	std::string metadata_;
	int16_t error_code_;
};

// format: [TopicName [Partition Offset Metadata ErrorCode]]
class OffsetFetchResponse: public Response {
public:
	OffsetFetchResponse(char **buf);
	virtual ~OffsetFetchResponse() {}

	virtual int CountSize();
	virtual void PrintAll();
	int ParseOffset(std::unordered_map<int, long> &partition_offset);
	int16_t GetErrorCode();
	
private:
	// XXX: format
	std::string topic_;
	std::vector<std::shared_ptr<PartitionOffsetME>> partitions_info_;
};

#endif
