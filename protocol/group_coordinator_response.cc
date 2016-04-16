#include "group_coordinator_response.h"
#include "util.h"
#include "easylogging++.h"

GroupCoordinatorResponse::GroupCoordinatorResponse(char **buf)
	: Response(ApiKey::GroupCoordinatorType, buf)
{
	error_code_ = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	coordinator_id_ = Util::NetBytesToInt(*buf); 
	(*buf) += 4;
	int16_t host_size = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	coordinator_host_ = std::string(*buf, host_size);
	(*buf) += host_size;
	coordinator_port_ = Util::NetBytesToInt(*buf); 

	if (Response::GetTotalSize() != CountSize())
	{
		throw "total size != count size are not equal";
	}
}

int GroupCoordinatorResponse::CountSize()
{
	int size = Response::CountSize();
	size += 2 + 4 + 2 + coordinator_host_.length() + 4;
	return size;
}

void GroupCoordinatorResponse::PrintAll()
{
	LOG(DEBUG) << "-----GroupCoordinatorResponse-----";
	Response::PrintAll();
	LOG(DEBUG) << "error code = " << error_code_;
	LOG(DEBUG) << "coordinator id = " << coordinator_id_;
	LOG(DEBUG) << "coordinator host = " << coordinator_host_;
	LOG(DEBUG) << "coordinator port = " << coordinator_port_;
	LOG(DEBUG) << "----------------------------------";
}

int32_t GroupCoordinatorResponse::GetCoordinatorId()
{
	return coordinator_id_;
}
