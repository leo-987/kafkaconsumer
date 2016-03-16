#ifndef _OFFSET_FETCH_REQUEST_H_
#define _OFFSET_FETCH_REQUEST_H_

#include <vector>
#include <string>

#include "request.h"
#include "request_response_type.h"

class OffsetFetchRequest: public Request {
public:
	OffsetFetchRequest(const std::string &group,
			const std::string &topic, const std::vector<int> &partitions, int correlation_id = ApiKey::OffsetFetchType);
	virtual ~OffsetFetchRequest() {}

	virtual int CountSize();
	virtual void PrintAll();
	virtual void Package(char **buf);

	// XXX: ConsumerGroup [TopicName [Partition]]
	std::string group_;
	std::string topic_;
	std::vector<int> partitions_;
};

#endif
