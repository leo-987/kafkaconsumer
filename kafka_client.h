#ifndef _KAFKA_CLIENT_H_
#define _KAFKA_CLIENT_H_

#include <map>
#include <deque>


class KafkaClient {
public:



	std::map<std::string, deque<Request>> in_flight_requests_;
};

#endif
