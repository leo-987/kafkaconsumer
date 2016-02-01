#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <cstdlib>
#include <cstdio>
#include <cstddef>
#include <string>

#include "net_util.h"

namespace NetUtil {

#define D(exp, fmt, ...) do {                 \
	if(!(exp)){                               \
		fprintf(stderr, fmt, ##__VA_ARGS__);  \
		abort();                              \
	}                                         \
}while(0)

static void SetNonBlock(int fd)
{
	fcntl(fd, F_SETFL, fcntl(fd,F_GETFL) | O_NONBLOCK);
}

static void SetReuseAddr(int fd)
{
	int ok = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(ok));
}

static void SetAddress(const char* ip, int port, struct sockaddr_in* addr)
{
	bzero(addr, sizeof(*addr));
	addr->sin_family = AF_INET;
	inet_pton(AF_INET, ip, &(addr->sin_addr));
	addr->sin_port=htons(port);
}

std::string AddressToString(struct sockaddr_in* addr)
{
	char ip[128];
	inet_ntop(AF_INET, &(addr->sin_addr), ip, sizeof(ip));
	char port[32];
	snprintf(port, sizeof(port), "%d", ntohs(addr->sin_port));
	std::string r;
	r = r + "(" + ip + ":" + port + ")";
	return r;
}

int NewTcpServer(int port)
{
	int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	D(fd > 0,"socket failed(%m)\n");
	SetNonBlock(fd);
	SetReuseAddr(fd);
	sockaddr_in addr;
	SetAddress("0.0.0.0", port, &addr);
	bind(fd, (struct sockaddr*)&addr, sizeof(addr));
	listen(fd, 64); // backlog = 64
	return fd;
}

int NewTcpClient(const char* ip, int port)
{
	int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SetNonBlock(fd);
	sockaddr_in addr;
	SetAddress(ip, port, &addr);
	connect(fd, (struct sockaddr*)(&addr), sizeof(addr));
	return fd;
}

};

