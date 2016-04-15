#include <string.h>

#include "response.h"
#include "util.h"
#include "easylogging++.h"

//------------------------------Head
//Response::Response(short api_key, int correlation_id)
//{
//	api_key_ = api_key;
//	total_size_ = 0;
//	correlation_id_ = correlation_id;
//}

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
	LOG(DEBUG) << "api key = " << api_key_;
	LOG(DEBUG) << "total size = " << total_size_;
	LOG(DEBUG) << "correlation id = " << correlation_id_;
}

int32_t Response::GetTotalSize()
{
	return total_size_;
}

void Response::SetTotalSize(int32_t total_size)
{
	total_size_ = total_size;
}
