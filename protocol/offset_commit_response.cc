#include "offset_commit_response.h"
#include "util.h"
#include "easylogging++.h"


PartitionE::PartitionE(char **buf)
{
	partition_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;

	error_code_ = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	LOG_IF(error_code_ != 0, ERROR) << "OffsetCommitResponse error code = " << error_code_;
}

int PartitionE::CountSize()
{
	return 4 + 2;
}

void PartitionE::PrintAll()
{
	LOG(DEBUG) << "partition = " << partition_;
	LOG(DEBUG) << "commit offset error code = " << error_code_;
}

int16_t PartitionE::GetErrorCode()
{
	return error_code_;
}

//------------------------------------------------------
TopicPartitionE::TopicPartitionE(char **buf)
{
	short topic_len = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	topic_ = std::string(*buf, topic_len);
	(*buf) += topic_len;

	int array_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int i = 0; i < array_size; i++)
	{
		PartitionE p(buf);
		partitions_.push_back(p);
	}
}

int TopicPartitionE::CountSize()
{
	int size = 2 + topic_.length();
	size += 4;
	for (auto p_it = partitions_.begin(); p_it != partitions_.end(); ++p_it)
		size += p_it->CountSize();
	return size;
}

void TopicPartitionE::PrintAll()
{
	LOG(DEBUG) << "topic = " << topic_;
	for (auto p_it = partitions_.begin(); p_it != partitions_.end(); ++p_it)
		p_it->PrintAll();
}

//------------------------------------------------------
OffsetCommitResponse::OffsetCommitResponse(char **buf)
	: Response(ApiKey::OffsetCommitType, buf)
{
	int array_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int i = 0; i < array_size; i++)
	{
		TopicPartitionE tp(buf);
		topic_partitions_.push_back(tp);
	}

	if (Response::GetTotalSize() != CountSize())
	{
		LOG(ERROR) << "total size != count size are not equal";
		throw;
	}
}

int OffsetCommitResponse::CountSize()
{
	int size = Response::CountSize();
	size += 4;
	for (auto tp_it = topic_partitions_.begin(); tp_it != topic_partitions_.end(); ++tp_it)
		size += tp_it->CountSize();
	return size;
}

void OffsetCommitResponse::PrintAll()
{
	LOG(DEBUG) << "------------OffsetCommitResponse-------------";
	Response::PrintAll();
	for (auto tp_it = topic_partitions_.begin(); tp_it != topic_partitions_.end(); ++tp_it)
		tp_it->PrintAll();
	LOG(DEBUG) << "---------------------------------------------";
}

int16_t OffsetCommitResponse::GetErrorCode()
{
	std::vector<PartitionE> &partitions = topic_partitions_[0].partitions_;
	for (auto p_it = partitions.begin(); p_it != partitions.end(); ++p_it)
	{
		int16_t error_code = p_it->GetErrorCode();
		if (error_code != 0)
			return error_code;
	}
	return 0;
}




