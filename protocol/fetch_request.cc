#include <arpa/inet.h>
#include "fetch_request.h"
#include "request_response_type.h"
#include "easylogging++.h"

PartitionFM::PartitionFM(int32_t partition, int64_t fetch_offset, int32_t max_bytes)
{
	partition_ = partition;
	fetch_offset_ = fetch_offset;
	max_bytes_ = max_bytes;
}

int PartitionFM::CountSize()
{
	return 4 + 8 + 4;
}

void PartitionFM::PrintAll()
{
	LOG(DEBUG) << "partition = " << partition_;
	LOG(DEBUG) << "fetch offset = " << fetch_offset_;
	LOG(DEBUG) << "max bytes = " << max_bytes_;
}

void PartitionFM::Package(char **buf)
{
	int partition = htonl(partition_);
	memcpy(*buf, &partition, 4);
	(*buf) += 4;

	// for Linux
	long fetch_offset = htobe64(fetch_offset_);
	// for Mac
	//long fetch_offset = htonll(fetch_offset_);
	memcpy(*buf, &fetch_offset, 8);
	(*buf) += 8;

	int max_bytes = htonl(max_bytes_);
	memcpy(*buf, &max_bytes, 4);
	(*buf) += 4;
}

//--------------------------------------------
TopicPartitionFM::TopicPartitionFM(const std::string &topic, const std::vector<PartitionFM> &partitions)
{
	topic_ = topic;
	partitions_ = partitions;
}

TopicPartitionFM::TopicPartitionFM(const std::string &topic, int32_t partition, int64_t offset)
{
	topic_ = topic;
	partitions_.push_back({partition, offset});
}

int TopicPartitionFM::CountSize()
{
	int size = 2 + topic_.length();
	size += 4;
	for (auto p_it = partitions_.begin(); p_it != partitions_.end(); ++p_it)
		size += p_it->CountSize();
	return size;
}

void TopicPartitionFM::PrintAll()
{
	LOG(DEBUG) << "topic name = " << topic_;
	for (auto p_it = partitions_.begin(); p_it != partitions_.end(); ++p_it)
		p_it->PrintAll();
}

void TopicPartitionFM::Package(char **buf)
{
	short topic_len = htons((short)topic_.length());
	memcpy(*buf, &topic_len, 2);
	(*buf) += 2;
	memcpy(*buf, topic_.c_str(), topic_.length());
	(*buf) += topic_.length();

	int array_size = htonl(partitions_.size());
	memcpy(*buf, &array_size, 4);
	(*buf) += 4;
	for (auto p_it = partitions_.begin(); p_it != partitions_.end(); ++p_it)
		p_it->Package(buf);
}

//------------------------------------
FetchRequest::FetchRequest(const std::string &topic, const std::vector<PartitionFM> &partitions, int correlation_id)
	: Request(ApiKey::FetchType, correlation_id, ApiVersion::v1)
{
	replica_id_ = -1;
	max_wait_time_ = 500;
	min_bytes_ = 1024;
	topic_partitions_.push_back({topic, partitions});
	total_size_ = CountSize();
}

FetchRequest::FetchRequest(const std::string &topic, int32_t partition, int64_t offset, int correlation_id)
	: Request(ApiKey::FetchType, correlation_id, ApiVersion::v1)
{
	replica_id_ = -1;
	max_wait_time_ = 500;
	min_bytes_ = 1024;
	topic_partitions_.push_back({topic, partition, offset});
	total_size_ = CountSize();
}

int FetchRequest::CountSize()
{
	int size = Request::CountSize();
	size += 4 + 4 + 4;
	size += 4;
	for (auto tp_it = topic_partitions_.begin(); tp_it != topic_partitions_.end(); ++tp_it)
		size += tp_it->CountSize();
	return size;
}

void FetchRequest::PrintAll()
{
	LOG(DEBUG) << "-----FetchRequest-----";
	Request::PrintAll();
	LOG(DEBUG) << "replica id = " << replica_id_;
	LOG(DEBUG) << "max wait time = " << max_wait_time_;
	LOG(DEBUG) << "min bytes = " << min_bytes_;
	for (auto tp_it = topic_partitions_.begin(); tp_it != topic_partitions_.end(); ++tp_it)
		tp_it->PrintAll();
	LOG(DEBUG) << "----------------------";
}

void FetchRequest::Package(char **buf)
{
	Request::Package(buf);

	// replica id
	int replica_id = htonl(replica_id_);
	memcpy(*buf, &replica_id, 4);
	(*buf) += 4;

	// max wait time
	int max_wait_time = htonl(max_wait_time_);
	memcpy(*buf, &max_wait_time, 4);
	(*buf) += 4;

	// min bytes
	int min_bytes = htonl(min_bytes_);
	memcpy(*buf, &min_bytes, 4);
	(*buf) += 4;

	int array_size = htonl(topic_partitions_.size());
	memcpy(*buf, &array_size, 4);
	(*buf) += 4;
	for (auto tp_it = topic_partitions_.begin(); tp_it != topic_partitions_.end(); ++tp_it)
		tp_it->Package(buf);
}

