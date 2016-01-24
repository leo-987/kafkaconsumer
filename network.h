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

	pthread_t tid_;

	struct ev_loop *loop_;
	std::vector<ev_io> watchers_;
	std::vector<int> fds_;

	ev_async async_watcher_;
};

#endif
