#ifndef _METADATA_REQUEST_H_
#define _METADATA_REQUEST_H_

#include <vector>

#include "request.h"
#include "request_response_type.h"

// format: [TopicName]
class MetadataRequest: public Request {
public:
	// For all topics
	MetadataRequest(int correlation_id = ApiKey::MetadataType);
	MetadataRequest(const std::vector<std::string> &topic_names, int correlation_id = ApiKey::MetadataType);

	virtual ~MetadataRequest() {}
	virtual int CountSize();
	virtual void PrintAll();
	virtual void Package(char **buf);

	std::vector<std::string> topics_;
};
#endif
