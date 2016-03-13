#include <iostream>
#include <string.h>

#include "response.h"
#include "util.h"

//------------------------------Head
Response::Response(short api_key, int correlation_id)
{
	api_key_ = api_key;
	total_size_ = 0;
	correlation_id_ = correlation_id;
}

Response::Response(short api_key, char **buf)
{
	api_key_ = api_key;
	total_size_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	correlation_id_ = Util::NetBytesToInt(*buf); 
	(*buf) += 4;
}

int Response::CountSize()
{
	return 4;
}

void Response::PrintAll()
{
	std::cout << "api key = " << api_key_ << std::endl;
	std::cout << "total size = " << total_size_ << std::endl;
	std::cout << "correlation id = " << correlation_id_ << std::endl;
}

