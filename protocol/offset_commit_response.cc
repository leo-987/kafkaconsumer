#include <iostream>

#include "offset_commit_response.h"
#include "util.h"


PartitionE::PartitionE(char **buf)
{
	partition_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;

	error_code_ = Util::NetBytesToShort(*buf);
	(*buf) += 2;
}

int PartitionE::CountSize()
{
	return 4 + 2;
}

void PartitionE::PrintAll()
{
	std::cout << "partition = " << partition_ << std::endl;
	std::cout << "error code = " << error_code_ << std::endl;
}
	

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
	std::cout << "topic = " << topic_ << std::endl;
	for (auto p_it = partitions_.begin(); p_it != partitions_.end(); ++p_it)
		p_it->PrintAll();
}

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

	if (total_size_ != CountSize())
	{
		throw "total size != count size are not equal";
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
	std::cout << "------------OffsetCommitResponse-------------" << std::endl;
	Response::PrintAll();
	for (auto tp_it = topic_partitions_.begin(); tp_it != topic_partitions_.end(); ++tp_it)
		tp_it->PrintAll();
	std::cout << "---------------------------------------------" << std::endl;
}


