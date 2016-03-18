#include <iostream>

#include "fetch_request.h"
#include "request_response_type.h"

FetchRequest::FetchRequest(const std::string &topic_name, int partition, long fetch_offset, int correlation_id)
	: Request(ApiKey::FetchType, correlation_id, ApiVersion::v1)
{
	replica_id_ = -1;
	max_wait_time_ = 500;
	min_bytes_ = 1024;
	topic_name_ = topic_name;
	partition_ = partition;
	fetch_offset_ = fetch_offset;
	max_bytes_ = 1048576;
	total_size_ = CountSize();
}

int FetchRequest::CountSize()
{
	int size = Request::CountSize();
	size += 4 + 4 + 4 +
		    4 + 2 + topic_name_.length() +
		    4 + 4 + 8 + 4;
	return size;
}

void FetchRequest::PrintAll()
{
	std::cout << "-----FetchRequest-----" << std::endl;
	Request::PrintAll();
	std::cout << "replica id = " << replica_id_ << std::endl;
	std::cout << "max wait time = " << max_wait_time_ << std::endl;
	std::cout << "min bytes = " << min_bytes_ << std::endl;
	std::cout << "topic name = " << topic_name_ << std::endl;
	std::cout << "partition = " << partition_ << std::endl;
	std::cout << "fetch offset = " << fetch_offset_ << std::endl;
	std::cout << "max bytes = " << max_bytes_ << std::endl;
	std::cout << "----------------------" << std::endl;
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

	// array size
	int topic_partition_size = htonl(1);
	memcpy(*buf, &topic_partition_size, 4);
	(*buf) += 4;

	// topic name
	short topic_name_size = htons((short)topic_name_.length());
	memcpy(*buf, &topic_name_size, 2);
	(*buf) += 2;
	memcpy(*buf, topic_name_.c_str(), topic_name_.length());
	(*buf) += topic_name_.length();

	// array size
	int partition_size = htonl(1);
	memcpy(*buf, &partition_size, 4);
	(*buf) += 4;

	// partition
	int partition = htonl(partition_);
	memcpy(*buf, &partition, 4);
	(*buf) += 4;

	// min bytes
	//long fetch_offset = htobe64(fetch_offset_);
	long fetch_offset = htonll(fetch_offset_);
	memcpy(*buf, &fetch_offset, 8);
	(*buf) += 8;

	// max bytes 
	int max_bytes = htonl(max_bytes_);
	memcpy(*buf, &max_bytes, 4);
	(*buf) += 4;
}

