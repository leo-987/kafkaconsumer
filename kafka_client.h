#ifndef _KAFKA_CLIENT_H_
#define _KAFKA_CLIENT_H_

#include <map>
#include <deque>

#include "network.h"
#include "request.h"
#include "response.h"
#include "blocking_queue.h"

class KafkaClient {
public:
	KafkaClient();
	~KafkaClient();

	int Init();
	int Start();

	Network *network_;
	std::map<int, std::deque<Request *>> in_flight_requests_;

	BlockingQueue<Request*> send_queue_;
	BlockingQueue<Response*> receive_queue_;
};

#endif
