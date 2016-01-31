
#include "node.h"

Node::Node(int fd, int id, const std::string &host, int port)
{
	fd_ = fd;
	id_ = id;
	host_ = host;
	port_ = port;
}
