#include <arpa/inet.h>
#include "offset_request.h"
#include "easylogging++.h"

PartitionTime::PartitionTime(int32_t partition, int64_t time, int32_t max_number)
{
	partition_ = partition;
	time_ = time;
	max_number_of_offsets_ = max_number;
}

int PartitionTime::CountSize()
{
	return 4 + 8 + 4;
}

void PartitionTime::PrintAll()
{
	LOG(DEBUG) << "partition = " << partition_;
	LOG(DEBUG) << "time = " << time_;
	LOG(DEBUG) << "max number of offsets = " << max_number_of_offsets_;
}

void PartitionTime::Package(char **buf)
{
	int32_t partition = htonl(partition_);
	memcpy(*buf, &partition, 4);
	(*buf) += 4;

	// for Mac
	//int64_t time = htonll(time_);
	// for Linux
	int64_t time = be64toh(time_);
	memcpy(*buf, &time, 8);
	(*buf) += 8;

	int32_t max_number_of_offsets = htonl(max_number_of_offsets_);
	memcpy(*buf, &max_number_of_offsets, 4);
	(*buf) += 4;
}

//------------------------------------
TopicPartition::TopicPartition(const std::string &topic, const std::vector<int32_t> &partitions, int64_t time)
{
	topic_ = topic;
	
	for (auto p_it = partitions.begin(); p_it != partitions.end(); ++p_it)
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
	LOG(DEBUG) << "topic = " << topic_;
	for (auto pt_it = partition_time_array_.begin(); pt_it != partition_time_array_.end(); ++pt_it)
		pt_it->PrintAll();
}

void TopicPartition::Package(char **buf)
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
}

//------------------------------------
OffsetRequest::OffsetRequest(const std::string &topic,
		const std::vector<int32_t> &partitions, int64_t time, int32_t replica_id, int correlation_id)
	: Request(ApiKey::OffsetType, correlation_id)
{
	replica_id_ = replica_id;

	TopicPartition topic_partition(topic, partitions, time);
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
	LOG(DEBUG) << "-----OffsetRequest-----";
	Request::PrintAll();
	LOG(DEBUG) << "replica id = " << replica_id_;
	for (auto tp_it = topic_partition_array_.begin(); tp_it != topic_partition_array_.end(); ++tp_it)
		tp_it->PrintAll();
	LOG(DEBUG) << "-----------------------";
}

void OffsetRequest::Package(char **buf)
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
}



