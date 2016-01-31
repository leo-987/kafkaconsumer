#ifndef _KAFKA_CLIENT_H_
#define _KAFKA_CLIENT_H_

#include <map>
#include <deque>

#include "network.h"
#include "request.h"
#include "response.h"
#include "node.h"

class KafkaClient {
public:
	KafkaClient();
	~KafkaClient();

	int Init();
	int Start();
	int PushRequest(Node *node, Request *request);
	short PopResponse(Node *node, Response **response);

	Network *network_;

	std::map<std::string, Node*> nodes_;
};

#endif
