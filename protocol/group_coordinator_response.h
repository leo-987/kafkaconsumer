#ifndef _GROUP_COORDINATOR_RESPONSE_H_
#define _GROUP_COORDINATOR_RESPONSE_H_

#include <string>
#include "response.h"

class GroupCoordinatorResponse: public Response {
public:
	GroupCoordinatorResponse(int correlation_id, short error_code,
		int coordinator_id, const std::string &coordinator_host, int coordinator_port);

	GroupCoordinatorResponse(char **buf);

	virtual int CountSize();
	virtual void PrintAll();

	short error_code_;
	int coordinator_id_;
	std::string coordinator_host_;
	int coordinator_port_;
};



#endif
