#include <iostream>

#include "offset_request.h"
#include "request_response_type.h"

PartitionTime::PartitionTime(int32_t p, int64_t t, int32_t m)
{
	partition_ = p;
	time_ = t;
	max_number_of_offsets_ = m;
}

int PartitionTime::CountSize()
{
	return 4 + 8 + 4;
}

void PartitionTime::PrintAll()
{
	std::cout << "partition = " << partition_ << std::endl;
	std::cout << "time = " << time_ << std::endl;
	std::cout << "max number of offsets = " << max_number_of_offsets_ << std::endl;
}

int PartitionTime::Package(char **buf)
{
	int32_t partition = htonl(partition_);
	memcpy(*buf, &partition, 4);
	(*buf) += 4;

	int64_t time = htonll(time_);
	memcpy(*buf, &time, 8);
	(*buf) += 8;

	int32_t max_number_of_offsets = htonl(max_number_of_offsets_);
	memcpy(*buf, &max_number_of_offsets, 4);
	(*buf) += 4;

	return 0;
}

//------------------------------------
TopicPartition::TopicPartition(const std::string &topic, const std::vector<int32_t> &p, int64_t time)
{
	topic_ = topic;
	
	for (auto p_it = p.begin(); p_it != p.end(); ++p_it)
	{
		PartitionTime partition_time(*p_it, time);
		partition_time_array_.push_back(partition_time);
	}
}

int TopicPartition::CountSize()
{
	int size = 0;
	size += 2 + topic_.length();
	size += 4;
	for (auto pt_it = partition_time_array_.begin(); pt_it != partition_time_array_.end(); ++pt_it)
	{
		size += pt_it->CountSize();
	}
	return size;
}

void TopicPartition::PrintAll()
{
	std::cout << "topic = " << topic_ << std::endl;
	for (auto pt_it = partition_time_array_.begin(); pt_it != partition_time_array_.end(); ++pt_it)
	{
		pt_it->PrintAll();
	}
}

int TopicPartition::Package(char **buf)
{
	short topic_len = htons((short)topic_.length());
	memcpy(*buf, &topic_len, 2);
	(*buf) += 2;
	memcpy(*buf, topic_.c_str(), topic_.length());
	(*buf) += topic_.length();

	int32_t array_size = htonl(partition_time_array_.size());
	memcpy(*buf, &array_size, 4);
	(*buf) += 4;
	for (auto pt_it = partition_time_array_.begin(); pt_it != partition_time_array_.end(); ++pt_it)
	{
		pt_it->Package(buf);
	}
	return 0;
}

//------------------------------------
OffsetRequest::OffsetRequest(int correlation_id, const std::string &topic,
		const std::vector<int32_t> &p, int64_t time, int32_t replica_id)
	: Request(ApiKey::OffsetType, correlation_id)
{
	replica_id_ = replica_id;

	TopicPartition topic_partition(topic, p, time);
	topic_partition_array_.push_back(topic_partition);

	total_size_ = CountSize();
}

int OffsetRequest::CountSize()
{
	int size = Request::CountSize();
	size += 4;
	size += 4;
	for (auto tp_it = topic_partition_array_.begin(); tp_it != topic_partition_array_.end(); ++tp_it)
	{
		size += tp_it->CountSize();
	}
	return size;
}

void OffsetRequest::PrintAll()
{
	std::cout << "-----OffsetRequest-----" << std::endl;
	Request::PrintAll();
	std::cout << "replica id = " << replica_id_ << std::endl;
	for (auto tp_it = topic_partition_array_.begin(); tp_it != topic_partition_array_.end(); ++tp_it)
	{
		tp_it->PrintAll();
	}
	std::cout << "-----------------------" << std::endl;
}

int OffsetRequest::Package(char **buf)
{
	Request::Package(buf);

	int32_t replica_id = htonl(replica_id_);
	memcpy(*buf, &replica_id, 4);
	(*buf) += 4;

	int32_t array_size = htonl(topic_partition_array_.size());
	memcpy(*buf, &array_size, 4);
	(*buf) += 4;
	for (auto tp_it = topic_partition_array_.begin(); tp_it != topic_partition_array_.end(); ++tp_it)
	{
		tp_it->Package(buf);
	}
	return 0;
}



