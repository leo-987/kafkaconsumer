#ifndef _KAFKA_CLIENT_H_
#define _KAFKA_CLIENT_H_

#include <map>
#include <deque>

#include "network.h"
#include "request.h"
#include "response.h"
#include "node.h"
#include "state_machine.h"

class KafkaClient {
public:
	KafkaClient();
	~KafkaClient();

	int Init();
	int Start();
	int Stop();

	//int PushRequest(Node *node, Request *request);
	//short PopResponse(Node *node, Response **response);

	Network *network_;



private:
	static void SignalHandler(int signal);
	static bool run_;
};

#endif
