#ifndef _BROKER_H_
#define _BROKER_H_

#include <string>

class Broker {
public:
	Broker();
	Broker(int fd, int id, const std::string &ip, int port);
	Broker(char **buf);

	int CountSize();
	void PrintAll();

	int fd_;
	int id_;
	std::string ip_;
	int port_;
};

#endif
