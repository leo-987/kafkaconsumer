#include <iostream>

#include "group_coordinator_request.h"

GroupCoordinatorRequest::GroupCoordinatorRequest(const std::string &group_id, int correlation_id)
	: Request(ApiKey::GroupCoordinatorType, correlation_id)
{
	group_ = group_id;
	total_size_ = CountSize();
}

int GroupCoordinatorRequest::CountSize()
{
	return Request::CountSize() + 2 + group_.length();
}

void GroupCoordinatorRequest::PrintAll()
{
	std::cout << "-----GroupCoordinatorRequest-----" << std::endl;
	Request::PrintAll();
	std::cout << "group id = " << group_ << std::endl;
	std::cout << "---------------------------------" << std::endl;
}

void GroupCoordinatorRequest::Package(char **buf)
{
	Request::Package(buf);
	short group_len = htons((short)group_.length());
	memcpy(*buf, &group_len, 2);
	(*buf) += 2;
	memcpy(*buf, group_.c_str(), group_.length());
}

