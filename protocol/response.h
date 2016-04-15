#ifndef _RESPONSE_H_
#define _RESPONSE_H_

#include <cstdint>
#include "request_response_type.h"


//------------------------------Head
class Response {
public:
	//Response(short api_key, int correlation_id);
	Response(short api_key, char **buf);
	virtual ~Response() {};
	virtual int CountSize();
	virtual void PrintAll();

	int32_t GetTotalSize();
	void SetTotalSize(int32_t total_size);

private:
	int16_t api_key_;			// it's not a part of protocol 
	int32_t total_size_;		// exclude itself
	int32_t correlation_id_;
};



#endif

