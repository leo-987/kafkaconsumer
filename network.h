#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <map>
#include <vector>

#include "request.h"
#include "response.h"
#include "node.h"
#include "partition.h"

class KafkaClient;

class Network {
public:
	Network(KafkaClient *client, const std::string &broker_list, const std::string &topic, const std::string &group);
	~Network();

	int Start();
	int Stop();
	int ReceiveResponseHandler(Node *node, Response **response);
	int SendRequestHandler(Node *node, Request *request);
	int Receive(int fd, Response **res);
	int Send(int fd, Request *request);
	short GetApiKeyFromResponse(int correlation_id);
	int PartitionAssignment();
	int CompleteRead(int fd, char *buf); 

	int GetCorrelationIdFromRequest(Request *request);
	short GetApiKeyFromRequest(Request *request);

	KafkaClient *client_;

	std::vector<int> fds_;

	//StateMachine *state_machine_;

	// broker id -> Node
	std::map<int, Node*> nodes_;
	std::map<int, Partition> partitions_map_;
	std::vector<std::string> members_;
	std::map<std::string, std::vector<int>> member_partition_map_;
	std::map<int, long> partition_offset_map_;

	int generation_id_;
	std::string member_id_;

	int last_correlation_id_;
	short last_api_key_;
private:
	std::string topic_;
	std::string group_;
};

#endif
