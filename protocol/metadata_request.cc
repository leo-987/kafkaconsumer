#include <arpa/inet.h>
#include "metadata_request.h"
#include "easylogging++.h"

MetadataRequest::MetadataRequest(const std::vector<std::string> &topics, int correlation_id)
	: Request(ApiKey::MetadataType, correlation_id)
{
	topics_ = topics;
	total_size_ = CountSize();
}

MetadataRequest::MetadataRequest(int correlation_id)
	: Request(ApiKey::MetadataType, correlation_id)
{
	total_size_ = CountSize();
}

int MetadataRequest::CountSize()
{
	int size = Request::CountSize();
	size += 4;
	for (auto t_it = topics_.begin(); t_it != topics_.end(); ++t_it)
		size += 2 + t_it->length();
	return size;
}

void MetadataRequest::PrintAll()
{
	LOG(DEBUG) << "-----MetadataRequest-----";
	Request::PrintAll();
	for (auto t_it = topics_.begin(); t_it != topics_.end(); ++t_it)
		LOG(DEBUG) << "topic name = " << *t_it;
	LOG(DEBUG) << "-------------------------";
}

void MetadataRequest::Package(char **buf)
{
	Request::Package(buf);
	
	int array_size = htonl(topics_.size());
	memcpy(*buf, &array_size, 4);
	(*buf) += 4;
	for (auto t_it = topics_.begin(); t_it != topics_.end(); ++t_it)
	{
		short topic_len = htons((short)t_it->length());
		memcpy(*buf, &topic_len, 2);
		(*buf) += 2;
		memcpy(*buf, t_it->c_str(), t_it->length());
		(*buf) += t_it->length();
	}
}
