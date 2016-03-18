#ifndef _GROUP_COORDINATOR_REQUEST_H_
#define _GROUP_COORDINATOR_REQUEST_H_

#include <string>

#include "request.h"
#include "request_response_type.h"

class GroupCoordinatorRequest: public Request {
public:
	GroupCoordinatorRequest(const std::string &group_id, int correlation_id = ApiKey::GroupCoordinatorType);
	virtual ~GroupCoordinatorRequest() {}

	virtual int CountSize();
	virtual void PrintAll();
	virtual void Package(char **buf);

	std::string group_;
};
#endif
