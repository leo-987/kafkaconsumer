#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <map>
#include <ev.h>

#include "request.h"
#include "response.h"
#include "blocking_queue.h"

class KafkaClient;

class Network {
public:
	Network();
	~Network();

	int Init(KafkaClient *client, const std::string &broker_list);
	int Start();
	int Stop();

	KafkaClient *client_;

	pthread_t event_loop_tid_;
	struct ev_loop *event_loop_;
	std::vector<int> socket_fds_;
	std::vector<ev_io> io_watchers_;
	ev_async async_watcher_;

	pthread_mutex_t queue_mutex_;

	std::map<int, std::deque<Request*>> in_flight_requests_;
	std::map<int, BlockingQueue<Request*>> send_queues_;
	std::map<int, BlockingQueue<Response*>> receive_queues_;
};

#endif
