#ifndef _HEARTBEAT_REQUEST_H_
#define _HEARTBEAT_REQUEST_H_

#include <string>

#include "request.h"
#include "request_response_type.h"

class HeartbeatRequest: public Request {
public:
	HeartbeatRequest(const std::string &group_id, int generation_id,
					 const std::string &member_id, int correlation_id = ApiKey::HeartbeatType);
	virtual ~HeartbeatRequest() {}

	virtual int CountSize();
	virtual void PrintAll();
	virtual void Package(char **buf);

	std::string group_id_;
	int generation_id_;
	std::string member_id_;
};

#endif
