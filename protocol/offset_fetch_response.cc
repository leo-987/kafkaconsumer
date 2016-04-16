#include <unordered_map>
#include "offset_fetch_response.h"
#include "easylogging++.h"

PartitionOffsetME::PartitionOffsetME(char **buf)
{
	partition_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;

	int64_t offset;
	memcpy(&offset, *buf, 8);
	offset_ = be64toh(offset);
	//offset_ = ntohll(offset);
	(*buf) += 8;

	int16_t metadata_length = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	metadata_ = std::string(*buf, metadata_length);
	(*buf) += metadata_length;

	error_code_ = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	LOG_IF(error_code_ != 0, ERROR) << "OffsetFetchResponse error code = " << error_code_;
}

int PartitionOffsetME::CountSize()
{
	return 4 + 8 + 2 + metadata_.length() + 2;
}

void PartitionOffsetME::PrintAll()
{
	LOG(DEBUG) << "partition  = " << partition_;
	LOG(DEBUG) << "offset     = " << offset_;
	LOG(DEBUG) << "metadata   = " << metadata_;
	LOG(DEBUG) << "error code = " << error_code_;
}

int16_t PartitionOffsetME::GetErrorCode()
{
	return error_code_;
}

int32_t PartitionOffsetME::GetPartition()
{
	return partition_;
}

int64_t PartitionOffsetME::GetOffset()
{
	return offset_;
}

//-----------------------------------------------
OffsetFetchResponse::OffsetFetchResponse(char **buf)
	: Response(ApiKey::OffsetFetchType, buf)
{
	// TODO: topics_array_size not used
	int32_t array_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;

	int16_t topic_length = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	topic_ = std::string(*buf, topic_length);
	(*buf) += topic_length;

	int32_t partitions_array_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int32_t i = 0; i < partitions_array_size; i++)
	{
		std::shared_ptr<PartitionOffsetME> partition_info = std::make_shared<PartitionOffsetME>(buf);
		partitions_info_.push_back(partition_info);
	}

	if (Response::GetTotalSize() != CountSize())
	{
		LOG(ERROR) << "total size != count size are not equal";
		throw;
	}
}

int OffsetFetchResponse::CountSize()
{
	int32_t size = Response::CountSize();
	size += 4 + 2 + topic_.length();

	// array
	size += 4;
	for (auto po_it = partitions_info_.begin(); po_it != partitions_info_.end(); ++po_it)
	{
		size += (*po_it)->CountSize();
	}
	return size;
}

void OffsetFetchResponse::PrintAll()
{
	LOG(DEBUG) << "-----OffsetFetchResponse-----";
	Response::PrintAll();
	LOG(DEBUG) << "topic = " << topic_;
	for (auto po_it = partitions_info_.begin(); po_it != partitions_info_.end(); ++po_it)
		(*po_it)->PrintAll();
	LOG(DEBUG) << "-----------------------------";
}

int OffsetFetchResponse::ParseOffset(std::unordered_map<int, long> &partition_offset)
{
	for (auto po_it = partitions_info_.begin(); po_it != partitions_info_.end(); ++po_it)
	{
		if ((*po_it)->GetErrorCode() != 0)
		{
			LOG(ERROR) << "error code = " << (*po_it)->GetErrorCode();
			continue;
		}

		int32_t partition = (*po_it)->GetPartition();
		int64_t offset = (*po_it)->GetOffset();
		partition_offset[partition] = offset;
	}
	return 0;
}

int16_t OffsetFetchResponse::GetErrorCode()
{
	for (auto po_it = partitions_info_.begin(); po_it != partitions_info_.end(); ++po_it)
	{
		int16_t error_code = (*po_it)->GetErrorCode();
		if (error_code != 0)
		{
			LOG(ERROR) << "error code = " << error_code;
			return error_code;
		}
	}
	return 0;
}




