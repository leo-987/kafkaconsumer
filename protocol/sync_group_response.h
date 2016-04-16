#ifndef _SYNC_GROUP_RESPONSE_H_
#define _SYNC_GROUP_RESPONSE_H_

#include <memory>
#include "response.h"
#include "member_assignment.h"

class SyncGroupResponse: public Response {
public:
	SyncGroupResponse(char **buf);

	virtual ~SyncGroupResponse() {}
	virtual int CountSize();
	virtual void PrintAll();
	void ParsePartitions(std::vector<int> &output_partitions);
	int16_t GetErrorCode();

	int16_t error_code_;
	std::shared_ptr<MemberAssignment> member_assignment_;	// bytes
};

#endif
