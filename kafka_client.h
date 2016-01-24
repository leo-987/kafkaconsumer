#ifndef _KAFKA_CLIENT_H_
#define _KAFKA_CLIENT_H_

#include <map>
#include <deque>

#include "network.h"
#include "request.h"


class KafkaClient {
public:
	KafkaClient();
	~KafkaClient();

	std::map<int, std::deque<Request *>> in_flight_requests_;
	Network *network_;

};

#endif
