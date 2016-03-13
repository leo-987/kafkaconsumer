#ifndef _HEARTBEAT_RESPONSE_H_
#define _HEARTBEAT_RESPONSE_H_

#include "response.h"

class HeartbeatResponse: public Response {
public:
	HeartbeatResponse(char **buf);

	virtual int CountSize();
	virtual void PrintAll();

	short error_code_;
};

#endif
