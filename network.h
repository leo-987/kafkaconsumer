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
	DISCOVER_COORDINATOR,
	JOIN_WITH_EMPTY_CONSUMER_ID,
	SYNC_GROUP,
	HEARTBEAT,
	SENDER_STOPPED,
};

class Network {
public:
	Network(KafkaClient *client, const std::string &broker_list, const std::string &topic, const std::string &group);
	~Network();

	int Start();
	int Stop();
	int ReceiveResponseHandler(Broker *broker, Response **response);
	int SendRequestHandler(Broker *broker, Request *request);
	int Receive(int fd, Response **res);
	int Send(int fd, Request *request);
	short GetApiKeyFromResponse(int correlation_id);
	int PartitionAssignment();
	int CompleteRead(int fd, char *buf); 

	std::map<std::string, std::vector<int>> member_partition_map_;
	std::map<int, long> partition_offset_map_;

	typedef int (Network::*StateProc)(Event &event);
	StateProc current_state_;
	Event event_;

	// state functions
	int Initial(Event &event);
	int DiscoverCoordinator(Event &event);
	int JoinGroup(Event &event);
	int SyncGroup(Event &event);
	int PartOfGroup(Event &event);

private:
	KafkaClient *client_;

	int last_correlation_id_;
	short last_api_key_;

	std::string topic_;
	std::string group_;

	// broker id -> Broker 
	std::map<int, Broker> brokers_;

	// partition id -> Partition
	std::map<int, Partition> partitions_;

	Broker *coordinator_;

	int generation_id_;
	std::string member_id_;

	bool amIGroupLeader_;
	std::vector<std::string> members_;
};

#endif
