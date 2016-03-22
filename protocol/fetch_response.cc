#include <iostream>

#include "fetch_response.h"
#include "util.h"

// Partition ErrorCode HighwaterMarkOffset MessageSetSize MessageSet
PartitionInfo::PartitionInfo(char **buf)
{
	partition_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;

	error_code_ = Util::NetBytesToShort(*buf);
	(*buf) += 2;

	long net_high_water_mark;
	memcpy(&net_high_water_mark, *buf, 8);
	// for Linux
	//high_water_mark_offset_ = be64toh(net_high_water_mark);
	// for Mac
	high_water_mark_offset_ = ntohll(net_high_water_mark);
	(*buf) += 8;

	message_set_size_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;

	if (message_set_size_ != 0)
		message_set_ = MessageSet(buf, message_set_size_);
}

int PartitionInfo::CountSize()
{
	return 4 + 2 + 8 + 4 + message_set_.CountSize();
}

void PartitionInfo::PrintAll()
{
	std::cout << "partition = " << partition_ << std::endl;
	std::cout << "error code = " << error_code_ << std::endl;
	std::cout << "high water mark offset = " << high_water_mark_offset_ << std::endl;
	std::cout << "message set size = " << message_set_size_ << std::endl;
	message_set_.PrintAll();
}

// TopicName [Partition ErrorCode HighwaterMarkOffset MessageSetSize MessageSet]
TopicPartitionInfo::TopicPartitionInfo(char **buf)
{
	// topic name
	short topic_len = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	topic_ = std::string(*buf, topic_len);
	(*buf) += topic_len;

	// partitions
	int array_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int i = 0; i < array_size; i++)
	{
		PartitionInfo partition_info(buf);
		partitions_info_.push_back(partition_info);
	}
}

int TopicPartitionInfo::CountSize()
{
	int size = 0;
	size += 2 + topic_.length();

	size += 4;
	for (auto pi_it = partitions_info_.begin(); pi_it != partitions_info_.end(); ++pi_it)
	{
		size += pi_it->CountSize();
	}
	return size;
}

void TopicPartitionInfo::PrintAll()
{
	std::cout << "topic name = " << topic_ << std::endl;

	for (auto pi_it = partitions_info_.begin(); pi_it != partitions_info_.end(); ++pi_it)
	{
		pi_it->PrintAll();
	}
}

// [TopicName [Partition ErrorCode HighwaterMarkOffset MessageSetSize MessageSet]] ThrottleTime
FetchResponse::FetchResponse(char **buf)
	: Response(ApiKey::FetchType, buf)
{
	// what?
	throttle_time_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;

	int array_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int i = 0; i < array_size; i++)
	{
		TopicPartitionInfo topic_partition_info(buf);
		topic_partitions_.push_back(topic_partition_info);
	}

	//throttle_time_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;

	if (total_size_ != CountSize())
	{
		throw "CountSize are not equal";
	}

	has_message_ = CheckHasMessage();
	StoreLastOffsets();
}

int FetchResponse::CountSize()
{
	int size = Response::CountSize();

	for (auto tp_it = topic_partitions_.begin(); tp_it != topic_partitions_.end(); ++tp_it)
	{
		size += tp_it->CountSize();
	}

	size += 8;
	return size;
}

void FetchResponse::PrintAll()
{
	std::cout << "-----FetchResponse-----" << std::endl;
	Response::PrintAll();
	for (auto tp_it = topic_partitions_.begin(); tp_it != topic_partitions_.end(); ++tp_it)
	{
		tp_it->PrintAll();
	}
	std::cout << "throttle time = " << throttle_time_ << std::endl;
	std::cout << "-----------------------" << std::endl;

}

void FetchResponse::PrintTopicMsg()
{
	// XXX: we assume only one topic
	TopicPartitionInfo &tp = topic_partitions_[0];
	std::vector<PartitionInfo> &partitions_info = tp.partitions_info_;

	for (auto p_it = partitions_info.begin(); p_it != partitions_info.end(); ++p_it)
	{
		PartitionInfo &p = *p_it;
		if (p.message_set_size_ == 0)
			continue;

		MessageSet &msg = p.message_set_;
		//std::cout << "topic: " << tp.topic_ << std::endl;
		msg.PrintMsg();
	}
}

bool FetchResponse::CheckHasMessage()
{
	// XXX: we assume only one topic
	TopicPartitionInfo &tp = topic_partitions_[0];
	std::vector<PartitionInfo> &partitions_info = tp.partitions_info_;

	for (auto p_it = partitions_info.begin(); p_it != partitions_info.end(); ++p_it)
	{
		PartitionInfo &p = *p_it;
		if (p.message_set_size_ != 0)
			return true;
	}
	return false;
}

bool FetchResponse::HasMessage()
{
	return has_message_;
}

bool FetchResponse::HasMessage(int32_t partition)
{
	if (partition_last_offset_.find(partition) != partition_last_offset_.end())
		return true;
	else
		return false;
}

int64_t FetchResponse::GetLastOffset(int32_t partition)
{
	return partition_last_offset_[partition];
}

void FetchResponse::StoreLastOffsets()
{
	// XXX: we assume only one topic
	TopicPartitionInfo &tp = topic_partitions_[0];
	std::vector<PartitionInfo> &partitions_info = tp.partitions_info_;
	for (auto p_it = partitions_info.begin(); p_it != partitions_info.end(); ++p_it)
	{
		PartitionInfo &p = *p_it;
		if (p.message_set_size_ == 0)
			continue;

		MessageSet &msg = p_it->message_set_;
		partition_last_offset_[p.partition_] = msg.GetLastOffset();
	}
}



