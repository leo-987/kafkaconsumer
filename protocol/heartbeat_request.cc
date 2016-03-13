#include <iostream>

#include "heartbeat_request.h"
#include "request_response_type.h"

HeartbeatRequest::HeartbeatRequest(int correlation_id, const std::string &group_id,
		int generation_id, const std::string &member_id)
	: Request(ApiKey::HeartbeatType, correlation_id)
{
	group_id_ = group_id;
	generation_id_ = generation_id;
	member_id_ = member_id;

	total_size_ = CountSize();
}

int HeartbeatRequest::CountSize()
{
	int size = Request::CountSize();
	size += 2 + group_id_.length()+
			4 +
			2 + member_id_.length();

	return size;
}

void HeartbeatRequest::PrintAll()
{
	std::cout << "-----HeartbeatRequest-----" << std::endl;
	Request::PrintAll();
	std::cout << "group id = " << group_id_ << std::endl;
	std::cout << "generation id = " << generation_id_ << std::endl;
	std::cout << "member id = " << member_id_ << std::endl;
	std::cout << "-------------------------" << std::endl;
}

int HeartbeatRequest::Package(char **buf)
{
	Request::Package(buf);

	// group id
	short group_id_size = htons((short)group_id_.length());
	memcpy(*buf, &group_id_size, 2);
	(*buf) += 2;
	memcpy(*buf, group_id_.c_str(), group_id_.length());
	(*buf) += group_id_.length();

	// generation id
	int generation_id = htonl(generation_id_);
	memcpy(*buf, &generation_id, 4);
	(*buf) += 4;

	// member id
	short member_id_size = htons((short)member_id_.length());
	memcpy(*buf, &member_id_size, 2);
	(*buf) += 2;
	memcpy(*buf, member_id_.c_str(), member_id_.length());

	return 0;
}
