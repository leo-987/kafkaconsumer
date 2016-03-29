#include <map>
#include "offset_fetch_response.h"
#include "easylogging++.h"

PartitionOffsetInfo::PartitionOffsetInfo(char **buf)
{
	partition_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;

	long offset;
	memcpy(&offset, *buf, 8);
	offset_ = be64toh(offset);
	//offset_ = ntohll(offset);
	(*buf) += 8;

	short metadata_length = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	metadata_ = std::string(*buf, metadata_length);
	(*buf) += metadata_length;

	error_code_ = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	LOG_IF(error_code_ != 0, ERROR) << "OffsetFetchResponse error code = " << error_code_;
}

int PartitionOffsetInfo::CountSize()
{
	return 4 + 8 + 2 + metadata_.length() + 2;
}

void PartitionOffsetInfo::PrintAll()
{
	LOG(DEBUG) << "partition  = " << partition_;
	LOG(DEBUG) << "offset     = " << offset_;
	LOG(DEBUG) << "metadata   = " << metadata_;
	LOG(DEBUG) << "error code = " << error_code_;
}

OffsetFetchResponse::OffsetFetchResponse(char **buf)
	: Response(ApiKey::OffsetFetchType, buf)
{
	// TODO: topics_array_size not used
	int array_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;

	short topic_length = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	topic_ = std::string(*buf, topic_length);
	(*buf) += topic_length;

	int partitions_array_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int i = 0; i < partitions_array_size; i++)
	{
		PartitionOffsetInfo partition_info(buf);
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
	int size = Response::CountSize();
	size += 4 + 2 + topic_.length();

	// array
	size += 4;
	for (auto pi_it = partitions_info_.begin(); pi_it != partitions_info_.end(); ++pi_it)
	{
		size += pi_it->CountSize();
	}
	return size;
}

void OffsetFetchResponse::PrintAll()
{
	LOG(DEBUG) << "-----OffsetFetchResponse-----";
	Response::PrintAll();
	LOG(DEBUG) << "topic = " << topic_;
	for (auto pi_it = partitions_info_.begin(); pi_it != partitions_info_.end(); ++pi_it)
		pi_it->PrintAll();
	LOG(DEBUG) << "-----------------------------";
}

int OffsetFetchResponse::ParseOffset(std::map<int, long> &partition_offset)
{
	for (auto pi_it = partitions_info_.begin(); pi_it != partitions_info_.end(); ++pi_it)
	{
		if (pi_it->error_code_ != 0)
		{
			std::cout << pi_it->error_code_ << std::endl;
			exit(1);
			continue;
		}

		int partition = pi_it->partition_;
		long offset = pi_it->offset_;
		partition_offset[partition] = offset;
	}
	return 0;
}


