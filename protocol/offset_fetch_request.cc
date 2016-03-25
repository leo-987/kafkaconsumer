#include "offset_fetch_request.h"
#include "easylogging++.h"

OffsetFetchRequest::OffsetFetchRequest(const std::string &group,
			const std::string &topic, const std::vector<int> &partitions, int correlation_id)
	: Request(ApiKey::OffsetFetchType, correlation_id, ApiVersion::v1)
{
	group_ = group;
	topic_ = topic;
	partitions_ = partitions;
	total_size_ = CountSize();
}

int OffsetFetchRequest::CountSize()
{
	int size = Request::CountSize();
	size += 2 + group_.length();
	size += 4 + 2 + topic_.length();
	size += 4 + 4 * partitions_.size();
	return size;
}

void OffsetFetchRequest::PrintAll()
{
	LOG(DEBUG) << "-----OffsetFetchRequest-----";
	Request::PrintAll();
	LOG(DEBUG) << "group = " << group_;
	LOG(DEBUG) << "topic = " << topic_;
	for (auto p_it = partitions_.begin(); p_it != partitions_.end(); ++p_it)
	{
		LOG(DEBUG) << "partition = " << *p_it;
	}
	LOG(DEBUG) << "----------------------------";

}

void OffsetFetchRequest::Package(char **buf)
{
	Request::Package(buf);

	// group
	short group_size = htons((short)group_.length());
	memcpy(*buf, &group_size, 2);
	(*buf) += 2;
	memcpy(*buf, group_.c_str(), group_.length());
	(*buf) += group_.length();

	// topic array size
	int topic_partition_size = htonl(1);
	memcpy(*buf, &topic_partition_size, 4);
	(*buf) += 4;

	// topic
	short topic_length = htons((short)topic_.length());
	memcpy(*buf, &topic_length, 2);
	(*buf) += 2;
	memcpy(*buf, topic_.c_str(), topic_.length());
	(*buf) += topic_.length();

	// topic array size
	int partition_size = htonl(partitions_.size());
	memcpy(*buf, &partition_size, 4);
	(*buf) += 4;

	for (auto p_it = partitions_.begin(); p_it != partitions_.end(); ++p_it)
	{
		int partition = htonl(*p_it);
		memcpy(*buf, &partition, 4);
		(*buf) += 4;
	}
}

