#include <iostream>

#include "group_coordinator_request.h"
#include "request_response_type.h"

GroupCoordinatorRequest::GroupCoordinatorRequest(int correlation_id, const std::string &group_id)
	: Request(ApiKey::GroupCoordinatorType, correlation_id)
{
	group_id_ = group_id;
	total_size_ = CountSize();
}

int GroupCoordinatorRequest::CountSize()
{
	return Request::CountSize() +		// head
		   2 + group_id_.length();	// body
}

void GroupCoordinatorRequest::PrintAll()
{
	std::cout << "-----GroupCoordinatorRequest-----" << std::endl;
	Request::PrintAll();
	std::cout << "group id = " << group_id_ << std::endl;
	std::cout << "---------------------------------" << std::endl;
}

int GroupCoordinatorRequest::Package(char **buf)
{
	Request::Package(buf);

	// group id
	short group_id_size = htons((short)group_id_.length());
	memcpy(*buf, &group_id_size, 2);
	(*buf) += 2;
	memcpy(*buf, group_id_.c_str(), group_id_.length());

	return 0;
}

