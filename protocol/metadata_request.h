#ifndef _METADATA_REQUEST_H_
#define _METADATA_REQUEST_H_

#include <vector>

#include "request.h"

class MetadataRequest: public Request {
public:
	MetadataRequest(int correlation_id, const std::vector<std::string> &topic_names,
			bool for_all_topic = false);

	virtual int CountSize();
	virtual void PrintAll();
	virtual int Package(char **buf);

	std::vector<std::string> topic_names_;
};
#endif
