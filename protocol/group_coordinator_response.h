#ifndef _GROUP_COORDINATOR_RESPONSE_H_
#define _GROUP_COORDINATOR_RESPONSE_H_

#include <string>
#include "response.h"

// format: ErrorCode CoordinatorId CoordinatorHost CoordinatorPort
class GroupCoordinatorResponse: public Response {
public:
	GroupCoordinatorResponse(char **buf);
	virtual ~GroupCoordinatorResponse() {};

	virtual int CountSize();
	virtual void PrintAll();
	int32_t GetCoordinatorId();

	int16_t error_code_;
	int32_t coordinator_id_;
	std::string coordinator_host_;
	int32_t coordinator_port_;
};



#endif
