#include <arpa/inet.h>
#include "offset_commit_request.h"
#include "easylogging++.h"

PartitionOM::PartitionOM(int32_t partition, int64_t offset, std::string metadata)
{
	partition_ = partition;
	offset_ = offset;
	metadata_ = metadata;
}

int PartitionOM::CountSize()
{
	return 4 + 8 + 2 + metadata_.length();
}

void PartitionOM::PrintAll()
{
	LOG(DEBUG) << "partition = " << partition_;
	LOG(DEBUG) << "offset = " << offset_;
	LOG(DEBUG) << "metadata = " << metadata_;
}

void PartitionOM::Package(char **buf)
{
	int32_t partition = htonl(partition_);
	memcpy(*buf, &partition, 4);
	(*buf) += 4;

	// For Mac
	//int64_t offset = htonll(offset_);
	// For Linux 
	int64_t offset = htobe64(offset_);
	memcpy(*buf, &offset, 8);
	(*buf) += 8;

	short metadata_length = htons((short)metadata_.length());
	memcpy(*buf, &metadata_length, 2);
	(*buf) += 2;
	memcpy(*buf, metadata_.c_str(), metadata_.length());
	(*buf) += metadata_.length();
}

//-------------------------------------------------------
TopicPartitionOM::TopicPartitionOM(const std::string &topic, int32_t partition, int64_t offset)
{
	topic_ = topic;
	partitions_.push_back({partition, offset});
}

TopicPartitionOM::TopicPartitionOM(const std::string &topic, const std::vector<PartitionOM> &partitions)
{
	topic_ = topic;
	partitions_ = partitions;
}

int TopicPartitionOM::CountSize()
{
	int size = 2 + topic_.length();
	size += 4;
	for (auto p_it = partitions_.begin(); p_it != partitions_.end(); ++p_it)
		size += p_it->CountSize();
	return size;
}

void TopicPartitionOM::PrintAll()
{
	LOG(DEBUG) << "topic = " << topic_;
	for (auto p_it = partitions_.begin(); p_it != partitions_.end(); ++p_it)
		p_it->PrintAll();
}

void TopicPartitionOM::Package(char **buf)
{
	short topic_len = htons((short)topic_.length());
	memcpy(*buf, &topic_len, 2);
	(*buf) += 2;
	memcpy(*buf, topic_.c_str(), topic_.length());
	(*buf) += topic_.length();

	int32_t array_size = htonl(partitions_.size());
	memcpy(*buf, &array_size, 4);
	(*buf) += 4;
	for (auto p_it = partitions_.begin(); p_it != partitions_.end(); ++p_it)
		p_it->Package(buf);
}

//----------------------------------------
OffsetCommitRequest::OffsetCommitRequest(const std::string &group, int32_t group_generation_id, const std::string &consumer_id, 
		const std::string &topic, int32_t partition, int64_t offset, int correlation_id)
	: Request(ApiKey::OffsetCommitType, correlation_id, ApiVersion::v2)
{
	group_ = group;
	group_generation_id_ = group_generation_id;
	consumer_id_ = consumer_id;
	retention_time_ = -1;
	topic_partitions_.push_back({topic, partition, offset});
	total_size_ = CountSize();
}

OffsetCommitRequest::OffsetCommitRequest(const std::string &group, int32_t group_generation_id, const std::string &consumer_id, 
		const std::string &topic, const std::vector<PartitionOM> &partitions, int correlation_id)
	: Request(ApiKey::OffsetCommitType, correlation_id, ApiVersion::v2)
{
	group_ = group;
	group_generation_id_ = group_generation_id;
	consumer_id_ = consumer_id;
	retention_time_ = -1;
	topic_partitions_.push_back({topic, partitions});
	total_size_ = CountSize();
}

int OffsetCommitRequest::CountSize()
{
	int size = Request::CountSize();
	size += 2 + group_.length() + 4 + 2 + consumer_id_.length() + 8;
	size += 4;
	for (auto tp_it = topic_partitions_.begin(); tp_it != topic_partitions_.end(); ++tp_it)
		size += tp_it->CountSize();
	return size;
}

void OffsetCommitRequest::PrintAll()
{
	LOG(DEBUG) << "---------------OffsetCommitRequest--------------";
	Request::PrintAll();
	LOG(DEBUG) << "group = " << group_;
	LOG(DEBUG) << "group generation id = " << group_generation_id_;
	LOG(DEBUG) << "consumer id = " << consumer_id_;
	LOG(DEBUG) << "retention time = " << retention_time_;
	for (auto tp_it = topic_partitions_.begin(); tp_it != topic_partitions_.end(); ++tp_it)
		tp_it->PrintAll();
	LOG(DEBUG) << "------------------------------------------------";
}

void OffsetCommitRequest::Package(char **buf)
{
	Request::Package(buf);

	short group_len = htons((short)group_.length());
	memcpy(*buf, &group_len, 2);
	(*buf) += 2;
	memcpy(*buf, group_.c_str(), group_.length());
	(*buf) += group_.length();

	int32_t group_generation_id = htonl(group_generation_id_);
	memcpy(*buf, &group_generation_id, 4);
	(*buf) += 4;

	short consumer_id_len = htons((short)consumer_id_.length());
	memcpy(*buf, &consumer_id_len, 2);
	(*buf) += 2;
	memcpy(*buf, consumer_id_.c_str(), consumer_id_.length());
	(*buf) += consumer_id_.length();

	// For Mac
	//int64_t retention_time = htonll(retention_time_);
	// For Linux 
	int64_t retention_time = htobe64(retention_time_);
	memcpy(*buf, &retention_time, 8);
	(*buf) += 8;

	int32_t array_size = htonl(topic_partitions_.size());
	memcpy(*buf, &array_size, 4);
	(*buf) += 4;
	for (auto tp_it = topic_partitions_.begin(); tp_it != topic_partitions_.end(); ++tp_it)
		tp_it->Package(buf);
}

