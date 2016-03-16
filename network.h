#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <map>
#include <vector>

#include "request.h"
#include "response.h"
#include "broker.h"
#include "partition.h"

class KafkaClient;

enum class Event {
	STARTUP,
	JOIN_REQUEST_WITH_EMPTY_CONSUMER_ID,
	HEARTBEAT,
	SENDER_STOPPED,
};

class Network {
public:
	Network(KafkaClient *client, const std::string &broker_list, const std::string &topic, const std::string &group);
	~Network();

	int Start();
	int Stop();
	int ReceiveResponseHandler(Broker *node, Response **response);
	int SendRequestHandler(Broker *node, Request *request);
	int Receive(int fd, Response **res);
	int Send(int fd, Request *request);
	short GetApiKeyFromResponse(int correlation_id);
	int PartitionAssignment();
	int CompleteRead(int fd, char *buf); 

	KafkaClient *client_;

	//StateMachine *state_machine_;


	std::map<int, Partition> partitions_map_;
	std::vector<std::string> members_;
	std::map<std::string, std::vector<int>> member_partition_map_;
	std::map<int, long> partition_offset_map_;

	int generation_id_;
	std::string member_id_;

	typedef int (Network::*StateProc)(Event &event);
	StateProc current_state_;
	Event event_;

	// state functions
	int Initial(Event &event);
	int DiscoverCoordinator(Event &event);
	int PartOfGroup(Event &event);
	int StoppedConsumption(Event &event);

private:
	int last_correlation_id_;
	short last_api_key_;

	std::string topic_;
	std::string group_;
	// broker id -> Node
	std::map<int, Broker> brokers_;
};

#endif
