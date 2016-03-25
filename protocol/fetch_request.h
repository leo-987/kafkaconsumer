#ifndef _FETCH_REQUEST_H_
#define _FETCH_REQUEST_H_

#include <string>
#include <vector>

#include "request.h"

class PartitionFM {
public:
	PartitionFM(int32_t partition, int64_t offset, int32_t max_bytes = 1048576);

	int CountSize();
	void PrintAll();
	void Package(char **buf);

private:
	int32_t     partition_;
	int64_t     fetch_offset_;
	int32_t     max_bytes_;
};

class TopicPartitionFM {
public:
	TopicPartitionFM(const std::string &topic, const std::vector<PartitionFM> &partitions);
	TopicPartitionFM(const std::string &topic, int32_t partition, int64_t offset);

	int CountSize();
	void PrintAll();
	void Package(char **buf);

private:
	std::string topic_;
	std::vector<PartitionFM> partitions_;
};

class FetchRequest: public Request {
public:
	FetchRequest(const std::string &topic, const std::vector<PartitionFM> &partitions, int correlation_id = ApiKey::FetchType);
	FetchRequest(const std::string &topic, int32_t partition, int64_t offset, int correlation_id = ApiKey::FetchType);
	virtual ~FetchRequest() {}

	virtual int CountSize();
	virtual void PrintAll();
	virtual void Package(char **buf);

private:
	int32_t     replica_id_;
	int32_t     max_wait_time_;
	int32_t     min_bytes_;
	std::vector<TopicPartitionFM> topic_partitions_;
};

#endif
