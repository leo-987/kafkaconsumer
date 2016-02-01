#ifndef _NET_UTIL_H_
#define _NET_UTIL_H_

namespace NetUtil {

	std::string AddressToString(struct sockaddr_in *addr);

	int NewTcpServer(int port);

	int NewTcpClient(const char *ip, int port);

};

#endif
