#ifndef _KAFKA_CLIENT_H_
#define _KAFKA_CLIENT_H_

#include "network.h"
#include "request.h"
#include "response.h"
#include "node.h"
#include "state_machine.h"

class KafkaClient {
public:
	KafkaClient(const std::string &brokers, const std::string &topic, const std::string &group);
	~KafkaClient();

	int Start();
	int Stop();

	Network *network_;
private:
	static void SignalHandler(int signal);
	static bool run_;
};

#endif
