#ifndef _METADATA_RESPONSE_H_
#define _METADATA_RESPONSE_H_

#include <string>

#include "response.h"

class Broker {
public:
	Broker(int node_id, const std::string &host, int port);
	Broker(char **buf);

	int CountSize();
	void PrintAll();

	int node_id_;
	std::string host_;
	int port_;
};

class PartitionMetadata {
public:
	PartitionMetadata(short error_code, int partition_id, int leader,
		const std::vector<int> &replicas, const std::vector<int> &irs);

	PartitionMetadata(char **buf);

	int CountSize();
	void PrintAll();

	short partition_error_code_;
	int partition_id_;
	int leader_;
	std::vector<int> replicas_;		// array
	std::vector<int> isr_;			// array
};

class TopicMetadata {
public:
	TopicMetadata(short error_code, const std::string &topic_name,
		const std::vector<PartitionMetadata> &partition_metadata);

	TopicMetadata(char **buf);

	int CountSize();
	void PrintAll();

	short topic_error_code_;
	std::string topic_name_;
	std::vector<PartitionMetadata> partition_metadata_;
};

class MetadataResponse: public Response {
public:
	MetadataResponse(char **buf);

	virtual int CountSize();
	virtual void PrintAll();

	int GetBrokerIdFromHostname(const std::string &hostname);
		
	std::vector<Broker> brokers_;				// array
	std::vector<TopicMetadata> topic_metadata_;	// array
};

#endif
