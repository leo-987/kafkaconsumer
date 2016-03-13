#ifndef _REQUEST_H_
#define _REQUEST_H_

#include <string>

//------------------------------Head
class Request {
public:
	Request(short api_key, int correlation_id, short api_version = 0, const std::string client_id = "consumer-1");
	virtual ~Request() {}

	virtual int CountSize();
	virtual void PrintAll();
	virtual int Package(char **buf);

	int GetCorrelationId();
	short GetApiKey();

	int   total_size_;		// exclude itself
	short api_key_;
	short api_version_;
	int   correlation_id_;
	std::string client_id_;
};


#endif

