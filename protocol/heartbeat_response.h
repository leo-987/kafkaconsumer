#ifndef _HEARTBEAT_RESPONSE_H_
#define _HEARTBEAT_RESPONSE_H_

#include "response.h"

// format: ErrorCode
class HeartbeatResponse: public Response {
public:
	HeartbeatResponse(char **buf);
	virtual ~HeartbeatResponse() {}
	virtual int CountSize();
	virtual void PrintAll();

	int16_t GetErrorCode();

private:
	int16_t error_code_;
};

#endif
