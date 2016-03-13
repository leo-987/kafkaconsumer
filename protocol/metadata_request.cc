#include <iostream>

#include "metadata_request.h"
#include "request_response_type.h"

MetadataRequest::MetadataRequest(int correlation_id, const std::vector<std::string> &topic_names,
		bool for_all_topic)
	: Request(ApiKey::MetadataType, correlation_id)
{
	if (for_all_topic == false)
		topic_names_ = topic_names;
	total_size_ = CountSize();
}

int MetadataRequest::CountSize()
{
	int size = Request::CountSize();
	size += 4;
	for (unsigned int i = 0; i < topic_names_.size(); i++)
	{
		size += 2 + topic_names_[i].length();
	}

	return size;
}

void MetadataRequest::PrintAll()
{
	std::cout << "-----MetadataRequest-----" << std::endl;
	Request::PrintAll();
	for (unsigned int i = 0; i < topic_names_.size(); i++)
	{
		std::cout << "topic name = " << topic_names_[i] << std::endl;
	}
	std::cout << "-------------------------" << std::endl;
}

int MetadataRequest::Package(char **buf)
{
	Request::Package(buf);

	// topics array
	int topic_names_size = htonl(topic_names_.size());
	memcpy(*buf, &topic_names_size, 4);
	(*buf) += 4;

	for (auto tn_it = topic_names_.begin(); tn_it != topic_names_.end(); ++tn_it)
	{
		std::string &topic = *tn_it;
		short topic_size = htons((short)topic.length());
		memcpy(*buf, &topic_size, 2);
		(*buf) += 2;
		memcpy(*buf, topic.c_str(), topic.length());
		(*buf) += topic.length();
	}

	return 0;
}
