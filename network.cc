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

Network::Network()
{
}

Network::~Network()
{
	for (auto it = fds_.begin(); it != fds_.end(); ++it)
		close(*it);

	// delete nodes
	for (auto it = nodes_.begin(); it != nodes_.end(); ++it)
		delete it->second;

	pthread_mutex_destroy(&queue_mutex_);
}

int Network::Init(KafkaClient *client, const std::string &broker_list)
{
	pthread_mutex_init(&queue_mutex_, NULL);

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
		fds_.push_back(fd);
		Node *node = new Node(fd, -1, host, port);
		nodes_.insert({host, node});
	}

	//state_machine_ = new StateMachine(network_, nodes_);
	//state_machine_->Init();

	std::cout << "Network init OK!" << std::endl;

	return 0;
}

int Network::Start()
{
	//while(1)
	{
		std::vector<std::string> topics({"test"});
		MetadataRequest *metadata_request = new MetadataRequest(2, topics);
		GroupCoordinatorRequest *group_request = new GroupCoordinatorRequest(0, "group");
		JoinGroupRequest *join_request = new JoinGroupRequest(0, "group", "", topics);

		// Random select a node
		auto it = nodes_.begin();
		std::advance(it, rand() % nodes_.size());
		Node *node = it->second;

		node = nodes_["w-w1902.add.nbt.qihoo.net"];

		SendRequestHandler(node, join_request);
		ReceiveResponseHandler(node);
	}

	return 0;
}

int Network::Stop()
{
	return 0;
}

int Network::ReceiveResponseHandler(Node *node)
{
	Response *response;

	int ret = DoReceive(node->fd_, &response);
	if (ret == 0)
	{
		response->Print();
		delete response;

		delete last_request_;
	}

	return 0;
}

int Network::SendRequestHandler(Node *node, Request *request)
{
	DoSend(node->fd_, request);

	last_request_ = request;

	return 0;
}

int Network::DoReceive(int fd, Response **res)
{
	char buf[1024] = {0};
	int nread = read(fd, buf, 1024);
	std::cout << "nread = " << nread << std::endl;
	if (nread <= 0)
	{
		if (nread == 0)
			std::cout << "connection has been closed" << std::endl;
		else
			std::cout << "error occurred" << std::endl;

		close(fd);
		return -1;
	}

	//int response_size = Util::NetBytesToInt(buf);
	int correlation_id = Util::NetBytesToInt(buf + 4); 
	int api_key = GetApiKeyFromResponse(last_request_, correlation_id);
	if (api_key < 0)
	{
		// not match
		return -1;
	}

	char *p = buf;
	switch(api_key)
	{
		case ApiKey::MetadataRequest:
		{
			MetadataResponse *response = new MetadataResponse(&p);
			*res = response;
			break;
		}
		case ApiKey::GroupCoordinatorRequest:
		{

			GroupCoordinatorResponse *response = new GroupCoordinatorResponse(&p);
			*res = response;
			break;
		}
		case ApiKey::JoinGroupRequest:
		{
			JoinGroupResponse *response = new JoinGroupResponse(&p);
			*res = response;
			break;
		}
	}

	return 0;
}

int Network::DoSend(int fd, Request *request)
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

short Network::GetApiKeyFromResponse(Request *last_request, int correlation_id)
{
	if (last_request->correlation_id_ != correlation_id)
	{
		std::cout << "The correlation_id are not equal" << std::endl;
		return -1;
	}

	return last_request->api_key_;
}

