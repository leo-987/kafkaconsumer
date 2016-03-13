#ifndef _HEARTBEAT_REQUEST_H_
#define _HEARTBEAT_REQUEST_H_

#include <string>

#include "request.h"

class HeartbeatRequest: public Request {
public:
	HeartbeatRequest(int correlation_id, const std::string &group_id, int generation_id,
					 const std::string &member_id);

	virtual int CountSize();
	virtual void PrintAll();
	virtual int Package(char **buf);

	std::string group_id_;
	int generation_id_;
	std::string member_id_;
};

#endif
