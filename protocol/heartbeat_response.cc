#include <cstdint>
#include "heartbeat_response.h"
#include "util.h"
#include "easylogging++.h"

#define ELPP_DISABLE_DEBUG_LOGS

HeartbeatResponse::HeartbeatResponse(char **buf)
	: Response(ApiKey::HeartbeatType, buf)
{
	error_code_ = Util::NetBytesToShort(*buf);
	LOG_IF(error_code_ != 0, ERROR) << "HeartbeatResponse error code = " << error_code_;
}

int HeartbeatResponse::CountSize()
{
	return Response::CountSize() + 2;
}

void HeartbeatResponse::PrintAll()
{
	LOG(DEBUG) << "-----HeartbeatResponse-----";
	Response::PrintAll();
	LOG(DEBUG) << "heartbeat error code = " << error_code_;
	LOG(DEBUG) << "---------------------------";
}

int16_t HeartbeatResponse::GetErrorCode()
{
	return error_code_;
}
