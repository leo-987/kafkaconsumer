#ifndef _REQUEST_H_
#define _REQUEST_H_

#include <string>

#include "request_response_type.h"

//------------------------------Head
class Request {
public:
	Request(short api_key, int correlation_id, short api_version = ApiVersion::v0, const std::string client_id = "consumer-1");
	virtual ~Request() {}

	virtual int CountSize();
	virtual void PrintAll();
	virtual void Package(char **buf);

	int GetCorrelationId();
	short GetApiKey();

protected:
	int         total_size_;		// exclude itself
	short       api_key_;
	short       api_version_;
	int         correlation_id_;
	std::string client_id_;
};


#endif

