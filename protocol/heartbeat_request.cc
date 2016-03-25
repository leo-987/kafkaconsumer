#include "heartbeat_request.h"
#include "easylogging++.h"

HeartbeatRequest::HeartbeatRequest(const std::string &group,
		int generation_id, const std::string &member_id, int correlation_id)
	: Request(ApiKey::HeartbeatType, correlation_id)
{
	group_ = group;
	generation_id_ = generation_id;
	member_id_ = member_id;
	total_size_ = CountSize();
}

int HeartbeatRequest::CountSize()
{
	int size = Request::CountSize();
	size += 2 + group_.length() +
			4 +
			2 + member_id_.length();

	return size;
}

void HeartbeatRequest::PrintAll()
{
	LOG(DEBUG) << "-----HeartbeatRequest-----";
	Request::PrintAll();
	LOG(DEBUG) << "group id = "      << group_;
	LOG(DEBUG) << "generation id = " << generation_id_;
	LOG(DEBUG) << "member id = "     << member_id_;
	LOG(DEBUG) << "-------------------------";
}

void HeartbeatRequest::Package(char **buf)
{
	Request::Package(buf);

	// group id
	short group_id_len = htons((short)group_.length());
	memcpy(*buf, &group_id_len, 2);
	(*buf) += 2;
	memcpy(*buf, group_.c_str(), group_.length());
	(*buf) += group_.length();

	// generation id
	int generation_id = htonl(generation_id_);
	memcpy(*buf, &generation_id, 4);
	(*buf) += 4;

	// member id
	short member_id_len = htons((short)member_id_.length());
	memcpy(*buf, &member_id_len, 2);
	(*buf) += 2;
	memcpy(*buf, member_id_.c_str(), member_id_.length());
}
