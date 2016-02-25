#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <map>
#include <ev.h>

#include "request.h"
#include "response.h"
#include "blocking_queue.h"
#include "node.h"

class KafkaClient;

class Network {
public:
	Network();
	~Network();

	int Init(KafkaClient *client, const std::string &broker_list);
	int Start();
	int Stop();
	int ReceiveResponseHandler(Node *node);
	int SendRequestHandler(Node *node, Request *request);
	int DoReceive(int fd, Response **res);
	int DoSend(int fd, Request *request);
	short GetApiKeyFromResponse(Request *last_request, int correlation_id);

	KafkaClient *client_;

	pthread_t event_loop_tid_;
	std::vector<int> fds_;

	pthread_mutex_t queue_mutex_;

	//StateMachine *state_machine_;

	// hostname -> Node
	std::map<std::string, Node*> nodes_;

	Request *last_request_;

private:
	bool run_;
};

#endif
