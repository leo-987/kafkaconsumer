#ifndef _OFFSET_FETCH_REQUEST_H_
#define _OFFSET_FETCH_REQUEST_H_

#include <vector>
#include <string>

#include "request.h"

class OffsetFetchRequest: public Request {
public:
	OffsetFetchRequest(int correlation_id, const std::string &group,
			const std::string &topic, const std::vector<int> &partitions);
	virtual ~OffsetFetchRequest() {}

	virtual int CountSize();
	virtual void PrintAll();
	virtual int Package(char **buf);

	// XXX: ConsumerGroup [TopicName [Partition]]
	std::string group_;
	std::string topic_;
	std::vector<int> partitions_;
};

#endif
