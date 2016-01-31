#ifndef _NODE_H_
#define _NODE_H_

#include <string>

class Node {
public:
	Node(int fd, int id, const std::string &host, int port);

	int fd_;
	int id_;
	std::string host_;
	int port_;
};

#endif
