#include "broker.h"
#include "util.h"
#include "easylogging++.h"

Broker::Broker()
{
}

Broker::Broker(int fd, int id, const std::string &ip, int port)
{
	fd_ = fd;
	id_ = id;
	ip_ = ip;
	port_ = port;
}

Broker::Broker(char **buf)
{
	// node id
	id_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;

	// ip/host name
	short ip_len = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	ip_ = std::string(*buf, ip_len);
	(*buf) += ip_len;

	// port
	port_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;
}

int Broker::CountSize()
{
	return 4 + 2 + ip_.length() + 4;
}

void Broker::PrintAll()
{
	LOG(DEBUG) << "fd = " << fd_;
	LOG(DEBUG) << "id = " << id_;
	LOG(DEBUG) << "ip = " << ip_;
	LOG(DEBUG) << "port = " << port_;
}
