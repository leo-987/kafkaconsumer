#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <deque>

#include "kafka_client.h"
#include "network.h"
#include "util.h"
#include "response.h"
#include "net_util.h"


static int ReceiveResponse(int fd, Network *network, Response **response);
static int SendRequest(int fd, Request *request);

static void EventLoopAsyncCallback(struct ev_loop *loop, ev_async *w, int revents)
{
	// revents == EV_ASYNC
	ev_break(loop, EVBREAK_ALL);
}

static short GetApiKeyFromResponse(std::deque<Request *> &in_flight_requests, int correlation_id)
{
	if (in_flight_requests.empty())
	{
		std::cout << "in_flight_requests queue is empty" << std::endl;
		return -1;
	}

	Request *in_flight_request = in_flight_requests.front();
	if (in_flight_request->correlation_id_ != correlation_id)
	{
		std::cout << "The correlation_id are not equal" << std::endl;
		return -1;
	}

	return in_flight_request->api_key_;
}

static void ReadWriteReadyCallback(struct ev_loop *loop, ev_io *w, int events)
{
	Network *network = (Network *)ev_userdata(loop);

	//std::cout << "aaaaaaaaaa" << std::endl;
	//sleep(1);
#if 1
	if (events & EV_READ)
	{
		//std::cout << "read ready" << std::endl;
		Response *response;
		int ret = ReceiveResponse(w->fd, network, &response);
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

		// pop and delete request
		std::deque<Request *> &in_flight_requests = network->in_flight_requests_.at(w->fd);
		pthread_mutex_lock(&network->queue_mutex_);
		delete in_flight_requests.front();
		in_flight_requests.pop_front();
		pthread_mutex_unlock(&network->queue_mutex_);

		network->receive_queues_[w->fd].Push(response);
	}
	if (events & EV_WRITE)
	{
		Request *request = network->send_queues_[w->fd].Front(100);
		if (request == NULL)
			return;
		
		int ret = SendRequest(w->fd, request);
		if (ret < 0)
			return;	// If failed, do not pop this request

		network->send_queues_[w->fd].Pop(-1);

		// push request to in flight request queue
		pthread_mutex_lock(&network->queue_mutex_);
		network->in_flight_requests_[w->fd].push_back(request);
		pthread_mutex_unlock(&network->queue_mutex_);

		//ev_io_stop(loop, w);
		//ev_io_set(w, w->fd, EV_READ);
		//ev_io_start(loop, w);
	}
#endif
}

static int ReceiveResponse(int fd, Network *network, Response **res)
{
	char buf[1024] = {0};
	int nread = read(fd, buf, 1024);
	std::cout << "nread = " << nread << std::endl;
	if (nread <= 0)
	{
		return nread;
	}

	char *p = buf;
	int response_size = Util::NetBytesToInt(p);
	p += 4;
	int correlation_id = Util::NetBytesToInt(p); 
	p += 4;

	std::deque<Request *> &in_flight_requests = network->in_flight_requests_.at(fd);
	int api_key = GetApiKeyFromResponse(in_flight_requests, correlation_id);

	if (api_key < 0)
	{
		// not match
		return -1;
	}

	switch(api_key)
	{
		case ApiKey::GroupCoordinatorRequest:
		{
			short error_code = Util::NetBytesToShort(p);
			p += 2;
			int coordinator_id = Util::NetBytesToInt(p); 
			p += 4;
			short host_size = Util::NetBytesToShort(p);
			p += 2;
			std::string coordinator_host(p, host_size);
			p += host_size;
			int coordinator_port = Util::NetBytesToInt(p); 

			GroupCoordinatorResponse *response = new GroupCoordinatorResponse(
								api_key, correlation_id, error_code, coordinator_id,
								coordinator_host, coordinator_port);

			if (response_size != response->total_size_)
			{
				std::cerr << "Size are not equal..." << std::endl;
				delete response;
				break;
			}

			*res = response;

			break;
		}
		case ApiKey::JoinGroupRequest:
		{
			short error_code = Util::NetBytesToShort(p);
			p += 2;
			int generation_id = Util::NetBytesToInt(p);
			p += 4;

			// GroupProtocol
			short group_protocol_size = Util::NetBytesToShort(p);
			p += 2;
			std::string group_protocol(p, group_protocol_size);
			p += group_protocol_size;

			// LeaderId
			short leader_id_size = Util::NetBytesToShort(p);
			p += 2;
			std::string leader_id(p, leader_id_size);
			p += leader_id_size;

			// MemberId
			short member_id_size = Util::NetBytesToShort(p);
			p += 2;
			std::string member_id(p, member_id_size);
			p += member_id_size;

			// Members
			int members_size = Util::NetBytesToInt(p);
			p += 4;

			std::vector<Member> members;
			for (int i = 0; i < members_size; i++)
			{
				// MemberId
				short member_id_size = Util::NetBytesToShort(p);
				p += 2;
				std::string member_id(p, member_id_size);
				p += member_id_size;

				// MemberMetadata
				int member_metadata_size = Util::NetBytesToInt(p);
				p += 4;
				std::string member_metadata(p, member_metadata_size);
				p += member_metadata_size;

				Member member(member_id, member_metadata);
				members.push_back(member);
			}

			JoinGroupResponse *response = new JoinGroupResponse(api_key, correlation_id,
					error_code, generation_id, group_protocol, leader_id, member_id, members);

			if (response_size != response->total_size_)
			{
				std::cerr << "Size are not equal..." << std::endl;
				delete response;
				break;
			}

			*res = response;

			break;
		}
	}

	return nread;
}

static int SendRequest(int fd, Request *request)
{
	int packet_size = request->total_size_ + 4;
	char packet[1024];
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
		case ApiKey::MetadataRequest:
		{
			MetadataRequest *r = dynamic_cast<MetadataRequest*>(request);

			// topics array
			int topic_names_size = htonl(r->topic_names_.size());
			memcpy(p, &topic_names_size, 4);
			p += 4;

			for (unsigned int i = 0; i < r->topic_names_.size(); i++)
			{
				std::string &topic = r->topic_names_[i];
				short topic_size = htons((short)topic.length());
				memcpy(p, &topic_size, 2);
				p += 2;
				memcpy(p, topic.c_str(), topic.length());
				p += topic.length();
			}

			break;
		}
		case ApiKey::GroupCoordinatorRequest:
		{
			GroupCoordinatorRequest *r = dynamic_cast<GroupCoordinatorRequest*>(request);

			// group id
			short group_id_size = htons((short)r->group_id_.length());
			memcpy(p, &group_id_size, 2);
			p += 2;
			memcpy(p, r->group_id_.c_str(), r->group_id_.length());
			break;
		}

		case ApiKey::JoinGroupRequest:
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
			int group_protocols_size = htonl(r->group_protocols_.size());
			memcpy(p, &group_protocols_size, 4);
			p += 4;

			for (unsigned int i = 0; i < r->group_protocols_.size(); i++)
			{
				// assignment strategy
				GroupProtocol &gp = r->group_protocols_[i];
				short assignment_strategy_size = htons((short)gp.assignment_strategy_.length());
				memcpy(p, &assignment_strategy_size, 2);
				p += 2;
				memcpy(p, gp.assignment_strategy_.c_str(), gp.assignment_strategy_.length());
				p += gp.assignment_strategy_.length();

				// ProtocolMetadata bytes size
				int protocol_metadata_size = htonl(gp.protocol_metadata_.Size());
				memcpy(p, &protocol_metadata_size, 4);
				p += 4;

				// version
				short version = htons(gp.protocol_metadata_.version_);
				memcpy(p, &version, 2);
				p += 2;

				// topics
				int topics_size = htonl(gp.protocol_metadata_.subscription_.size());
				memcpy(p, &topics_size, 4);
				p += 4;

				for (unsigned int j = 0; j < gp.protocol_metadata_.subscription_.size(); j++)
				{
					std::string &topic = gp.protocol_metadata_.subscription_[j];
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

	int nwrite = write(fd, packet, packet_size);

	std::cout << "Send " << nwrite << " bytes" << std::endl;
	if (nwrite < 0)
	{
		if (errno == EWOULDBLOCK)
		{
			return 0;
		}
		else
		{
			std::cerr << "An error has occured while writing to the server." << std::endl;
			return -1;
		}
	}

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
	pthread_mutex_init(&queue_mutex_, NULL);
}

Network::~Network()
{
	for (unsigned int i = 0; i < socket_fds_.size(); i++)
	    close(socket_fds_[i]);

	ev_loop_destroy(event_loop_);

	// delete nodes
	for (auto it = client_->nodes_.begin(); it != client_->nodes_.end(); ++it)
		delete it->second;

	pthread_mutex_destroy(&queue_mutex_);
}

int Network::Init(KafkaClient *client, const std::string &broker_list)
{
	client_ = client;

	// create nodes
	std::vector<std::string> brokers = Util::Split(broker_list, ',');
	for (unsigned int i = 0; i < brokers.size(); i++)
	{
		std::vector<std::string> host_port = Util::Split(brokers[i], ':');
		std::string host = host_port[0];
		std::string ip = Util::HostnameToIp(host);
		int port = std::stoi(host_port[1]);
		int fd = NetUtil::NewTcpClient(ip.c_str(), port);
		socket_fds_.push_back(fd);

		Node *node = new Node(fd, -1, host, port);
		client->nodes_.insert({host, node});
	}

	// create loop
	event_loop_ = ev_loop_new(EVFLAG_AUTO);
	ev_set_userdata(event_loop_, this);

	// io watcher
	for (unsigned int i = 0; i < socket_fds_.size(); i++)
	{
		ev_io watcher;
    	ev_io_init(&watcher, ReadWriteReadyCallback, socket_fds_[i], EV_READ | EV_WRITE);
		io_watchers_.push_back(watcher);
	}

	// async watcher
	ev_async_init(&async_watcher_, EventLoopAsyncCallback);

	std::cout << "Network init OK!" << std::endl;

	return 0;
}

int Network::Start()
{
	// start io watcher
	for (unsigned int i = 0; i < io_watchers_.size(); i++)
		ev_io_start(event_loop_, &io_watchers_[i]);

	// start async watcher
	ev_async_start(event_loop_, &async_watcher_);

	pthread_create(&event_loop_tid_, NULL, EventLoopThread, event_loop_);
	std::cout << "Event loop thread created!" << std::endl;

	return 0;
}

int Network::Stop()
{
	for (unsigned int i = 0; i < io_watchers_.size(); i++)
		ev_io_stop(event_loop_, &io_watchers_[i]);

	if (ev_async_pending(&async_watcher_) == false)
		ev_async_send(event_loop_, &async_watcher_);

	pthread_join(event_loop_tid_, NULL);
	std::cout << "Network thread " << event_loop_tid_ << " exit!" << std::endl;

	return 0;
}


