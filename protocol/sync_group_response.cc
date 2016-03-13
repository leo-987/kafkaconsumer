#include <iostream>

#include "sync_group_response.h"
#include "util.h"

SyncGroupResponse::SyncGroupResponse(char **buf)
	: Response(ApiKey::SyncGroupType, buf)
{
	// error code
	error_code_ = Util::NetBytesToShort(*buf);
	(*buf) += 2;

	// MemberAssignment bytes
	int member_assignment_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;

	// XXX: we should use member_assignment_size
	member_assignment_ = MemberAssignment(buf);

	if (total_size_ != CountSize())
	{
		throw "CountSize are not equal";
	}
}

int SyncGroupResponse::CountSize()
{
	int size = Response::CountSize();
	size += 2;
	size += 4 + member_assignment_.CountSize();
	return size;
}

void SyncGroupResponse::PrintAll()
{
	std::cout << "-----SyncGroupResponse-----" << std::endl;
	Response::PrintAll();
	std::cout << "error code = " << error_code_ << std::endl;
	member_assignment_.PrintAll();
	std::cout << "---------------------------" << std::endl;
}
