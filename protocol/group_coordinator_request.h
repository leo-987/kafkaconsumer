#ifndef _GROUP_COORDINATOR_REQUEST_H_
#define _GROUP_COORDINATOR_REQUEST_H_

#include <string>

#include "request.h"

class GroupCoordinatorRequest: public Request {
public:
	GroupCoordinatorRequest(int correlation_id, const std::string &group_id);
	virtual ~GroupCoordinatorRequest() {}

	virtual int CountSize();
	virtual void PrintAll();
	virtual int Package(char **buf);

	std::string group_id_;
};
#endif
