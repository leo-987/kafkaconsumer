#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <ev.h>

#include "request.h"

class KafkaClient;

class Network {
public:
	Network();
	~Network();

	int Init(KafkaClient *client);
	int Start();
	int Stop();

	KafkaClient *client_;

	std::vector<pthread_t> tids_;

	std::vector<struct ev_loop *> loops_;
	std::vector<int> fds_;
	std::vector<ev_io> watchers_;
	std::vector<ev_async> async_watchers_;
};

#endif
