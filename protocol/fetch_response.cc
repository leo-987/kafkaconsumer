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
	std::cout << "message size = " << message_set_size_ << std::endl;
	message_set_.PrintAll();
}

// TopicName [Partition ErrorCode HighwaterMarkOffset MessageSetSize MessageSet]
TopicPartitionInfo::TopicPartitionInfo(char **buf)
{
	// topic name
	short topic_name_size = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	topic_ = std::string(*buf, topic_name_size);
	(*buf) += topic_name_size;

	// partitions
	int partitions_info_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int i = 0; i < partitions_info_size; i++)
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
	(*buf) += 4;

	int topics_info_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int i = 0; i < topics_info_size; i++)
	{
		TopicPartitionInfo topic_partition_info(buf);
		topic_partitions_.push_back(topic_partition_info);
	}

	throttle_time_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;

	if (total_size_ != CountSize())
	{
		throw "CountSize are not equal";
	}
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

void FetchResponse::PrintTopicAndMsg()
{
	// XXX: we assume only one topic
	TopicPartitionInfo &tp = topic_partitions_[0];
	PartitionInfo &p = tp.partitions_info_[0];
	MessageSet &msg = p.message_set_;

	std::cout << "topic: " << tp.topic_ << std::endl;
	msg.PrintMsg();
}

bool FetchResponse::IsEmptyMsg()
{
	MessageSet &ms = topic_partitions_[0].partitions_info_[0].message_set_;
	return ms.offset_message_.size() == 0;
}

int64_t FetchResponse::GetLastOffset()
{
	MessageSet &msg = topic_partitions_[0].partitions_info_[0].message_set_;
	return msg.GetLastOffset();
}



