#include <iostream>

#include "heartbeat_response.h"
#include "util.h"

HeartbeatResponse::HeartbeatResponse(char **buf)
	: Response(ApiKey::HeartbeatType, buf)
{
	// error code
	error_code_ = Util::NetBytesToShort(*buf);
}

int HeartbeatResponse::CountSize()
{
	return Response::CountSize() + 2;
}

void HeartbeatResponse::PrintAll()
{
	std::cout << "-----HeartbeatResponse-----" << std::endl;
	Response::PrintAll();
	std::cout << "error code = " << error_code_ << std::endl;
	std::cout << "---------------------------" << std::endl;
}

