#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <uv.h>
#include <ev.h>

#include "common.h"
#include "request.h"
#include "response.h"

enum class State {
	down,
	startup,
	joined,
};

State state = State::down;

std::vector<int> fds;

int net_bytes_to_int(char *buf)
{
	int a = buf[0] & 0xFF;
	a |= ((buf[1] << 8) & 0xFF00);
	a |= ((buf[2] << 16) & 0xFF0000);
	a |= ((buf[3] << 24) & 0xFF000000);
	return ntohl(a);
}


short net_bytes_to_short(char *buf)
{
	int a = buf[0] & 0xFF;
	a |= ((buf[1] << 8) & 0xFF00);
	return ntohs(a);
}


bool validate_correlation_id()
{

}


int send_request(int fd, Request &request)
{
	char *packet = new char[request.total_size_ + 4];
	char *p = packet;

	// total size
	int request_size = htonl(request.total_size_);
	memcpy(p, &request_size, 4);
	p += 4;

	// api key
	short api_key = htons(request.api_key_);
	memcpy(p, &api_key, 2);
	p += 2;

	// api version
	short api_version = htons(request.api_version_);
	memcpy(p, &api_version, 2);
	p += 2;

	// correlation id
	int correlation_id = htonl(request.correlation_id_);
	memcpy(p, &correlation_id, 4);
	p += 4;

	// client id
	short client_id_size = htons(request.client_id_size_);
	memcpy(p, &client_id_size, 2);
	p += 2;
	memcpy(p, request.client_id_.c_str(), request.client_id_size_);
	p += request.client_id_size_;

	switch(request.api_key_)
	{
		case 10:
		{
			GroupCoordinatorRequest &r = static_cast<GroupCoordinatorRequest&>(request);
			short group_id_size = htons(r.group_id_size_);
			memcpy(p, &group_id_size, 2);
			p += 2;
			memcpy(p, r.group_id_.c_str(), r.group_id_size_);
			break;
		}

		case 11:
		{
			JoinGroupRequest &r = static_cast<JoinGroupRequest&>(request);

			// group id
			short group_id_size = htons(r.group_id_size_);
			memcpy(p, &group_id_size, 2);
			p += 2;
			memcpy(p, r.group_id_.c_str(), r.group_id_size_);
			p += r.group_id_size_;

			// session timeout
			int session_timeout = htonl(r.session_timeout_);
			memcpy(p, &session_timeout, 4);
			p += 4;

			// member id
			short member_id_size = htons(r.member_id_size_);
			memcpy(p, &member_id_size, 2);
			p += 2;
			memcpy(p, r.member_id_.c_str(), r.member_id_size_);
			p += r.member_id_size_;

			// protocol type
			short protocol_type_size = htons(r.protocol_type_size_);
			memcpy(p, &protocol_type_size, 2);
			p += 2;
			memcpy(p, r.protocol_type_.c_str(), r.protocol_type_size_);
			p += r.protocol_type_size_;

			// array size
			int group_protocol_size = htonl(r.group_protocol_size_);
			memcpy(p, &group_protocol_size, 4);
			p += 4;

			// assignment strategy
			short assignment_strategy_size = htons(r.group_protocol_.assignment_strategy_size_);
			memcpy(p, &assignment_strategy_size, 2);
			p += 2;
			memcpy(p, r.group_protocol_.assignment_strategy_.c_str(), r.group_protocol_.assignment_strategy_size_);
			p += r.group_protocol_.assignment_strategy_size_;

			// version
			short version = htons(r.group_protocol_.protocol_metadata_.version_);
			memcpy(p, &version, 2);
			p += 2;

			// topic
			int topic_size = htonl(r.group_protocol_.protocol_metadata_.topics_size_);
			memcpy(p, &topic_size, 4);
			p += 4;

			for (int i = 0; i < r.group_protocol_.protocol_metadata_.topics_.size(); i++)
			{
				std::pair<short, std::string> size_and_topic = r.group_protocol_.protocol_metadata_.topics_[i];
				short size = htons(size_and_topic.first);
				memcpy(p, &size, 2);
				p += 2;
				memcpy(p, size_and_topic.second.c_str(), size_and_topic.first);
				p += size_and_topic.first;
			}

			// consumer id
			int user_data_size = htonl(r.group_protocol_.protocol_metadata_.user_data_size_);
			memcpy(p, &user_data_size, 4);
			p += 4;
			memcpy(p, r.group_protocol_.protocol_metadata_.user_data_,
				   r.group_protocol_.protocol_metadata_.user_data_size_);

			break;
		}
	}

	int n = write(fd, packet, request.total_size_ + 4);
	std::cout << n << std::endl;

	return 0;
}


int receive_response(int fd)
{
	char buf[1024] = {0};
	int n = read(fd, buf, 1024);
	std::cout << "n = " << n << std::endl;
	if (n <= 0)
	{
		return n;
	}

	char *p = buf;
	int response_size = net_bytes_to_int(p);
	p += 4;
	int correlation_id = net_bytes_to_int(p); 
	p += 4;

	switch(correlation_id)
	{
		case 0:
		{
			short error_code = net_bytes_to_short(p);
			p += 2;
			int coordinator_id = net_bytes_to_int(p); 
			p += 4;
			short host_size = net_bytes_to_short(p);
			p += 2;
			std::string coordinator_host(p, host_size);
			p += host_size;
			int coordinator_port = net_bytes_to_int(p); 

			GroupCoordinatorResponse response(correlation_id, error_code, coordinator_id,
					coordinator_host, coordinator_port);

			std::cout << "correlation id = " << response.correlation_id_ << std::endl;
			std::cout << "error code = " << response.error_code_ << std::endl;
			std::cout << "coordinator id = " << response.coordinator_id_ << std::endl;
			std::cout << "coordinator host = " << response.coordinator_host_ << std::endl;
			std::cout << "coordinator port = " << response.coordinator_port_ << std::endl;
				
			std::cout << response.total_size_ << std::endl;

			break;
		}
	}

	return n;
}

void read_write_cb(struct ev_loop *loop, ev_io *w, int events)
{
	if (events & EV_READ)
	{
		if (w->fd == fds[2])
		{
			int ret = receive_response(w->fd);
			if (ret <= 0)
			{
				if (ret == 0)
				{
					std::cout << "connection has been closed" << std::endl;
					ev_io_stop(loop, w);
					close(w->fd);
				}
				else if (errno != EAGAIN)
				{
					std::cout << "error occurred" << std::endl;
				}
			}
		}
	}
	else if (events & EV_WRITE)
	{
		if (w->fd == fds[2])
		{
			//GroupCoordinatorRequest request_1(0, "test");
			//send_request(w->fd, request_1);

			std::vector<std::string> topic({"test"});
			JoinGroupRequest request_2(1, "test", "", topic);
			send_request(w->fd, request_2);

			ev_io_stop(loop, w);
			ev_io_set(w, w->fd, EV_READ);
			ev_io_start(loop, w);
		}
	}

	//close(w->fd);
	//ev_break(reactor, EVBREAK_ALL);
}


int main()
{
	struct ev_loop *loop = EV_DEFAULT;

    int fd_b1 = common::new_tcp_client("10.123.81.11", 9092);
    int fd_b2 = common::new_tcp_client("10.123.81.12", 9092);
    int fd_b3 = common::new_tcp_client("10.123.81.13", 9092);
	
	fds.push_back(fd_b1);
	fds.push_back(fd_b2);
	fds.push_back(fd_b3);

	ev_io iow[3];
	for (int i = 0; i < 3; i++)
	{
    	ev_io_init(iow + i, read_write_cb, fds[i], EV_READ | EV_WRITE);
		ev_io_start(loop, iow + i);
	}

    ev_run(loop, 0);

	for (int i = 0; i < fds.size(); i++)
	{
	    close(fds[i]);
	}
    ev_loop_destroy(loop);

    return 0;
}
