#include <iostream>

#include "broker.h"
#include "util.h"

Broker::Broker(int fd, int id, const std::string &host, int port)
{
	fd_ = fd;
	node_id_ = id;
	host_ = host;
	port_ = port;
}

Broker::Broker(char **buf)
{
	// node id
	node_id_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;

	// host name
	short host_size = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	host_ = std::string(*buf, host_size);
	(*buf) += host_size;

	// port
	port_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;
}

int Broker::CountSize()
{
	return 4 + 2 + host_.length() + 4;
}

void Broker::PrintAll()
{
	std::cout << "fd = " << fd_ << std::endl;
	std::cout << "node id = " << node_id_ << std::endl;
	std::cout << "host = " << host_ << std::endl;
	std::cout << "port = " << port_ << std::endl;
}
