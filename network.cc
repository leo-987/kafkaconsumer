#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <deque>

#include "kafka_client.h"
#include "network.h"
#include "util.h"
#include "response.h"
#include "common.h"


static int ReceiveResponse(int fd, KafkaClient *client);
static int SendRequest(int fd, KafkaClient *client, Request *request);

static void EventLoopAsyncCallback(struct ev_loop *loop, ev_async *w, int revents)
{
	ev_break(loop, EVBREAK_ALL);
}

static int GetApiKeyFromResponse(std::deque<Request *> &in_flight_requests, int correlation_id)
{
	if (in_flight_requests.empty())
		return -1;

	Request *in_flight_request = in_flight_requests.front();
	if (in_flight_request->correlation_id_ != correlation_id)
		return -1;

	return in_flight_request->api_key_;
}

static void ReadWriteReadyCallback(struct ev_loop *loop, ev_io *w, int events)
{
	KafkaClient *client = (KafkaClient *)ev_userdata(loop);

	//std::cout << "aaaaaaaaaa" << std::endl;
	//sleep(1);
#if 1
	if (events & EV_READ)
	{
		//std::cout << "read ready" << std::endl;
		int ret = ReceiveResponse(w->fd, client);
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
	if (events & EV_WRITE)
	{
		// Get request from kafka client
		Request *request = client->send_queue_.Pop();
		if (request != NULL)
			SendRequest(w->fd, client, request);

		//std::vector<std::string> topic({"test"});
		//JoinGroupRequest request_2(1, "test", "", topic);
		//send_request(w->fd, request_2);

		//ev_io_stop(loop, w);
		//ev_io_set(w, w->fd, EV_READ);
		//ev_io_start(loop, w);
	}
#endif
}

static int ReceiveResponse(int fd, KafkaClient *client)
{
	char buf[1024] = {0};
	int nread = read(fd, buf, 1024);
	std::cout << "nread = " << nread << std::endl;
	if (nread <= 0)
	{
		return nread;
	}

	char *p = buf;
	int response_size = Util::net_bytes_to_int(p);
	std::cout << "response size = " << response_size << std::endl;
	p += 4;
	int correlation_id = Util::net_bytes_to_int(p); 
	std::cout << "correlation id = " << correlation_id << std::endl;
	p += 4;

	std::deque<Request *> in_flight_requests = client->in_flight_requests_.at(fd);
	int api_key = GetApiKeyFromResponse(in_flight_requests, correlation_id);

	if (api_key < 0)
	{
		// not match
		return -1;
	}

	// pop
	in_flight_requests.pop_front();

	switch(api_key)
	{
		case 10:
		{
			short error_code = Util::net_bytes_to_short(p);
			p += 2;
			int coordinator_id = Util::net_bytes_to_int(p); 
			p += 4;
			short host_size = Util::net_bytes_to_short(p);
			p += 2;
			std::string coordinator_host(p, host_size);
			p += host_size;
			int coordinator_port = Util::net_bytes_to_int(p); 

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
		case 11:
		{
			break;
		}
	}

	return nread;
}

static int SendRequest(int fd, KafkaClient *client, Request *request)
{
	char *packet = new char[request->total_size_ + 4];
	char *p = packet;

	// total size
	int request_size = htonl(request->total_size_);
	memcpy(p, &request_size, 4);
	p += 4;

	// api key
	short api_key = htons(request->api_key_);
	memcpy(p, &api_key, 2);
	p += 2;

	// api version
	short api_version = htons(request->api_version_);
	memcpy(p, &api_version, 2);
	p += 2;

	// correlation id
	int correlation_id = htonl(request->correlation_id_);
	memcpy(p, &correlation_id, 4);
	p += 4;

	// client id
	short client_id_size = htons((short)request->client_id_.length());
	memcpy(p, &client_id_size, 2);
	p += 2;
	memcpy(p, request->client_id_.c_str(), request->client_id_.length());
	p += request->client_id_.length();

	switch(request->api_key_)
	{
		case 10:
		{
			GroupCoordinatorRequest *r = dynamic_cast<GroupCoordinatorRequest*>(request);
			short group_id_size = htons((short)r->group_id_.length());
			memcpy(p, &group_id_size, 2);
			p += 2;
			memcpy(p, r->group_id_.c_str(), r->group_id_.length());
			break;
		}

		case 11:
		{
			JoinGroupRequest *r = dynamic_cast<JoinGroupRequest*>(request);

			// group id
			short group_id_size = htons((short)r->group_id_.length());
			memcpy(p, &group_id_size, 2);
			p += 2;
			memcpy(p, r->group_id_.c_str(), r->group_id_.length());
			p += r->group_id_.length();

			// session timeout
			int session_timeout = htonl(r->session_timeout_);
			memcpy(p, &session_timeout, 4);
			p += 4;

			// member id
			short member_id_size = htons((short)r->member_id_.length());
			memcpy(p, &member_id_size, 2);
			p += 2;
			memcpy(p, r->member_id_.c_str(), r->member_id_.length());
			p += r->member_id_.length();

			// protocol type
			short protocol_type_size = htons((short)r->protocol_type_.length());
			memcpy(p, &protocol_type_size, 2);
			p += 2;
			memcpy(p, r->protocol_type_.c_str(), r->protocol_type_.length());
			p += r->protocol_type_.length();

			// array size
			int group_protocol_size = htonl(r->group_protocols_.size());
			memcpy(p, &group_protocol_size, 4);
			p += 4;

			for (int i = 0; i < r->group_protocols_.size(); i++)
			{
				// assignment strategy
				GroupProtocol &gp = r->group_protocols_[i];
				short assignment_strategy_size = htons((short)gp.assignment_strategy_.length());
				memcpy(p, &assignment_strategy_size, 2);
				p += 2;
				memcpy(p, gp.assignment_strategy_.c_str(), gp.assignment_strategy_.length());
				p += gp.assignment_strategy_.length();

				// version
				short version = htons(gp.protocol_metadata_.version_);
				memcpy(p, &version, 2);
				p += 2;

				// topics
				int topics_size = htonl(gp.protocol_metadata_.subscription_.size());
				memcpy(p, &topics_size, 4);
				p += 4;

				for (int j = 0; j < gp.protocol_metadata_.subscription_.size(); j++)
				{
					std::string topic = gp.protocol_metadata_.subscription_[j];
					short topic_size = htons((short)topic.length());
					memcpy(p, &topic_size, 2);
					p += 2;
					memcpy(p, topic.c_str(), topic.length());
					p += topic.length();
				}

				// consumer id
				int user_data_size = htonl(gp.protocol_metadata_.user_data_.size());
				memcpy(p, &user_data_size, 4);
				p += 4;
				memcpy(p, gp.protocol_metadata_.user_data_.data(),
					   gp.protocol_metadata_.user_data_.size());
			}

			break;
		}
	}

	int n = write(fd, packet, request->total_size_ + 4);
	std::cout << "Send " << n << " bytes" << std::endl;

	// push request to in flight request queue
	client->in_flight_requests_[fd].push_back(request);

	return 0;
}

static void* EventLoopThread(void *arg)
{
	struct ev_loop *loop = (struct ev_loop *)arg;
    ev_run(loop, 0);
	return NULL;
}

//-----------------------------------Network
Network::Network()
{
	//loop_ = NULL;
}

Network::~Network()
{
	for (int i = 0; i < fds_.size(); i++)
	{
	    close(fds_[i]);
    	ev_loop_destroy(loops_[i]);
	}
}

int Network::Init(KafkaClient *client)
{
	client_ = client;

	//loop_ = EV_DEFAULT;

	int fd_b1 = common::new_tcp_client("10.123.81.11", 9092);
	int fd_b2 = common::new_tcp_client("10.123.81.12", 9092);
	int fd_b3 = common::new_tcp_client("10.123.81.13", 9092);

	fds_.push_back(fd_b1);
	fds_.push_back(fd_b2);
	fds_.push_back(fd_b3);

	loops_.resize(fds_.size());

	for (int i = 0; i < loops_.size(); i++)
	{
		loops_[i] = ev_loop_new(EVFLAG_AUTO);

		// associate KafkaClient with the loop
		ev_set_userdata(loops_[i], client);
	}

	//watchers_.resize(3);

	for (int i = 0; i < fds_.size(); i++)
	{
		// io watcher
		ev_io watcher;
    	ev_io_init(&watcher, ReadWriteReadyCallback, fds_[i], EV_READ | EV_WRITE);
		watchers_.push_back(watcher);
	
		// async wathcer
		ev_async async_watcher;
		ev_async_init(&async_watcher, EventLoopAsyncCallback);
		async_watchers_.push_back(async_watcher);
	}

	std::cout << "Network init OK!" << std::endl;
}

int Network::Start()
{
	for (int i = 0; i < loops_.size(); i++)
	{
		// start watchers
		ev_io_start(loops_[i], &watchers_[i]);
		ev_async_start(loops_[i], &async_watchers_[i]);

		// one loop per thread
		pthread_t tid;
		pthread_create(&tid, NULL, EventLoopThread, loops_[i]);
		tids_.push_back(tid);
	}

	std::cout << "Net thread created!" << std::endl;
}

int Network::Stop()
{
	for (int i = 0; i < loops_.size(); i++)
	{
		ev_io_stop(loops_[i], &watchers_[i]);

		if (ev_async_pending(&async_watchers_[i]) == false)
			ev_async_send(loops_[i], &async_watchers_[i]);
	
		pthread_join(tids_[i], NULL);
	}

	std::cout << "Network thread exit!" << std::endl;

	return 0;
}


