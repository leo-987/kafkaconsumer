#include <iostream>

#include "offset_response.h"
#include "util.h"

PartitionOffsets::PartitionOffsets(char **buf)
{
	partition_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	error_code_ = Util::NetBytesToShort(*buf);
	(*buf) += 2;

	int array_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int i = 0; i < array_size; i++)
	{
		long offset;
		memcpy(&offset, *buf, 8);
		// for Linux
		//high_water_mark_offset_ = be64toh(net_high_water_mark);
		// for Mac
		offset = ntohll(offset);
		(*buf) += 8;
		offset_array_.push_back(offset);
	}
}

int PartitionOffsets::CountSize()
{
	return 4 + 2 + 4 + offset_array_.size() * 8;
}

void PartitionOffsets::PrintAll()
{
	std::cout << "partition = " << partition_ << std::endl;
	std::cout << "error code = " << error_code_ << std::endl;
	for (auto o_it = offset_array_.begin(); o_it != offset_array_.end(); ++o_it)
	{
		std::cout << "offset = " << *o_it << std::endl;
	}
}


TopicPartitionOR::TopicPartitionOR(char **buf)
{
	short topic_len = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	topic_ = std::string(*buf, topic_len);
	(*buf) += topic_len;

	int array_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int i = 0; i < array_size; i++)
	{
		PartitionOffsets partition_offset(buf);
		partition_offset_array_.push_back(partition_offset);
	}
}

int TopicPartitionOR::CountSize()
{
	int size = 2 + topic_.length();
	size += 4;
	for (auto po_it = partition_offset_array_.begin(); po_it != partition_offset_array_.end(); ++po_it)
	{
		size += po_it->CountSize();
	}
	return size;
}

void TopicPartitionOR::PrintAll()
{
	std::cout << "topic = " << topic_ << std::endl;
	for (auto po_it = partition_offset_array_.begin(); po_it != partition_offset_array_.end(); ++po_it)
	{
		po_it->PrintAll();
	}
}

OffsetResponse::OffsetResponse(char **buf)
	: Response(ApiKey::OffsetType, buf)
{
	int array_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int i = 0; i < array_size; i++)
	{
		TopicPartitionOR tp(buf);
		topic_partition_array_.push_back(tp);
	}

	if (total_size_ != CountSize())
	{
		throw "CountSize are not equal";
	}
}

int OffsetResponse::CountSize()
{
	int size = Response::CountSize();

	size += 4;
	for (auto tp_it = topic_partition_array_.begin(); tp_it != topic_partition_array_.end(); ++tp_it)
	{
		size += tp_it->CountSize();
	}
	return size;
}

void OffsetResponse::PrintAll()
{
	std::cout << "-----OffsetResponse-----" << std::endl;
	Response::PrintAll();
	for (auto tp_it = topic_partition_array_.begin(); tp_it != topic_partition_array_.end(); ++tp_it)
	{
		tp_it->PrintAll();
	}
	std::cout << "------------------------" << std::endl;
}

long OffsetResponse::GetNewOffset()
{
	return topic_partition_array_[0].partition_offset_array_[0].offset_array_[0];
}


