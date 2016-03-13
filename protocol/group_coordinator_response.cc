#include <iostream>

#include "group_coordinator_response.h"
#include "util.h"

GroupCoordinatorResponse::GroupCoordinatorResponse(int correlation_id, short error_code,
		int coordinator_id, const std::string &coordinator_host, int coordinator_port)
	: Response(ApiKey::GroupCoordinatorType, correlation_id)
{
	error_code_ = error_code;
	coordinator_id_ = coordinator_id;
	coordinator_host_ = coordinator_host;
	coordinator_port_ = coordinator_port;
	total_size_ = CountSize();
}

GroupCoordinatorResponse::GroupCoordinatorResponse(char **buf)
	: Response(ApiKey::GroupCoordinatorType, buf)
{
	error_code_ = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	coordinator_id_ = Util::NetBytesToInt(*buf); 
	(*buf) += 4;
	short host_size = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	coordinator_host_ = std::string(*buf, host_size);
	(*buf) += host_size;
	coordinator_port_ = Util::NetBytesToInt(*buf); 

	if (total_size_ != CountSize())
	{
		throw "total size != count size are not equal";
	}
}

int GroupCoordinatorResponse::CountSize()
{
	int size = 0;
	size = Response::CountSize() + 2 /* error_code */ + 4 /* coordinator_id */+
		   2 /* coordinator_host size */ + coordinator_host_.length() + 4;
	return size;
}

void GroupCoordinatorResponse::PrintAll()
{
	std::cout << "-----GroupCoordinatorResponse-----" << std::endl;
	Response::PrintAll();
	std::cout << "error code = " << error_code_ << std::endl;
	std::cout << "coordinator id = " << coordinator_id_ << std::endl;
	std::cout << "coordinator host = " << coordinator_host_ << std::endl;
	std::cout << "coordinator port = " << coordinator_port_ << std::endl;
	std::cout << "----------------------------------" << std::endl;
}

