
#include "response.h"


//------------------------------Head
Response::Response(short api_key, int correlation_id)
{
	api_key_ = api_key;
	total_size_ = 0;
	correlation_id_ = correlation_id;
}


//------------------------------GroupCoordinatorResponse
GroupCoordinatorResponse::GroupCoordinatorResponse(short api_key, int correlation_id, short error_code,
		int coordinator_id, const std::string &coordinator_host, int coordinator_port)
	: Response(api_key, correlation_id)
{
	error_code_ = error_code;
	coordinator_id_ = coordinator_id;
	coordinator_host_ = coordinator_host;
	coordinator_port_ = coordinator_port;
	total_size_ = 4 + 2 + 4 + 2 + coordinator_host_.length() + 4;
}


//------------------------------JoinGroupResponse
#if 0
JoinGroupResponse::JoinGroupResponse(int correlation_id, short error_code, int generation_id,
		std::string group_protocol, std::string leader_id, std::string member_id,
		std::vector<Member> members)
	: Response(correlation_id)
{
	error_code_ = error_code;
	generation_id_ = generation_id;
	group_protocol_ = group_protocol;
	leader_id_ = leader_id;
	member_id_ = member_id;
	members_ = members;
}
#endif
