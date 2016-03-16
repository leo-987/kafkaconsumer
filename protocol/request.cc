#include <iostream>
#include <arpa/inet.h>
#include <string.h>

#include "request.h"
#include "member_assignment.h"

//------------------------------Head
Request::Request(short api_key, int correlation_id, short api_version, std::string client_id)
{
	total_size_ = 0;
	api_key_ = api_key;
	api_version_ = api_version;
	correlation_id_ = correlation_id;
	client_id_ = client_id;
}

int Request::GetCorrelationId()
{
	return correlation_id_;
}

short Request::GetApiKey()
{
	return api_key_;
}

int Request::CountSize()
{
	return 2 + 2 + 4 + 2 + client_id_.length();
}

void Request::PrintAll()
{
	std::cout << "total size = " << total_size_ << std::endl;
	std::cout << "api key = " << api_key_ << std::endl;
	std::cout << "api version = " << api_version_ << std::endl;
	std::cout << "correlation id = " << correlation_id_ << std::endl;
	std::cout << "client id = " << client_id_ << std::endl;
}

void Request::Package(char **buf)
{
	// total size
	int request_size = htonl(total_size_);
	memcpy(*buf, &request_size, 4);
	(*buf) += 4;

	// api key
	short api_key = htons(api_key_);
	memcpy(*buf, &api_key, 2);
	(*buf) += 2;

	// api version
	short api_version = htons(api_version_);
	memcpy(*buf, &api_version, 2);
	(*buf) += 2;

	// correlation id
	int correlation_id = htonl(correlation_id_);
	memcpy(*buf, &correlation_id, 4);
	(*buf) += 4;

	// client id
	short client_id_size = htons((short)client_id_.length());
	memcpy(*buf, &client_id_size, 2);
	(*buf) += 2;
	memcpy(*buf, client_id_.c_str(), client_id_.length());
	(*buf) += client_id_.length();
}

