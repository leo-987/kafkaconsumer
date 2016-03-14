#ifndef _BROKER_H_
#define _BROKER_H_

#include <string>

class Broker {
public:
	Broker(int fd, int id, const std::string &host, int port);
	Broker(char **buf);

	int CountSize();
	void PrintAll();

	int fd_;
	int node_id_;
	std::string host_;
	int port_;
};

#endif
