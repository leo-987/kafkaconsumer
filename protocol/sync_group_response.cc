#include "sync_group_response.h"
#include "util.h"
#include "easylogging++.h"

SyncGroupResponse::SyncGroupResponse(char **buf)
	: Response(ApiKey::SyncGroupType, buf)
{
	// error code
	error_code_ = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	if (error_code_ != 0)
	{
		LOG(ERROR) << "SyncGroupResponse error code = " << error_code_;
		throw 1;
	}

	// MemberAssignment bytes
	int member_assignment_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;

	// XXX: we should use member_assignment_size
	member_assignment_ = std::make_shared<MemberAssignment>(buf);

	if (Response::GetTotalSize() != CountSize())
	{
		LOG(ERROR) << "CountSize are not equal";
		throw;
	}
}

int SyncGroupResponse::CountSize()
{
	int size = Response::CountSize();
	size += 2;
	size += 4 + member_assignment_->CountSize();
	return size;
}

void SyncGroupResponse::PrintAll()
{
	LOG(DEBUG) << "-----SyncGroupResponse-----";
	Response::PrintAll();
	LOG(DEBUG) << "error code = " << error_code_;
	member_assignment_->PrintAll();
	LOG(DEBUG) << "---------------------------";
}

void SyncGroupResponse::ParsePartitions(std::vector<int> &output_partitions)
{
	member_assignment_->ParsePartitions(output_partitions);
}

int16_t SyncGroupResponse::GetErrorCode()
{
	return error_code_;
}
