#ifndef _SYNC_GROUP_RESPONSE_H_
#define _SYNC_GROUP_RESPONSE_H_

#include "response.h"
#include "member_assignment.h"

class SyncGroupResponse: public Response {
public:
	SyncGroupResponse(char **buf);

	virtual ~SyncGroupResponse() {}
	virtual int CountSize();
	virtual void PrintAll();

	short error_code_;
	MemberAssignment member_assignment_;	// bytes
};

#endif
