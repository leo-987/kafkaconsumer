#include "broker.h"
#include "util.h"
#include "easylogging++.h"

Broker::Broker()
{
}

Broker::Broker(int fd, int id, const std::string &host, int port)
{
	fd_ = fd;
	id_ = id;
	host_ = host;
	port_ = port;
}

Broker::Broker(char **buf)
{
	// node id
	id_ = Util::NetBytesToInt(*buf);
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
	LOG(DEBUG) << "fd = " << fd_;
	LOG(DEBUG) << "id = " << id_;
	LOG(DEBUG) << "host = " << host_;
	LOG(DEBUG) << "port = " << port_;
}
