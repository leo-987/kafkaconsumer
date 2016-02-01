#include <iostream>
#include <unistd.h>

#include "kafka_client.h"


KafkaClient::KafkaClient()
{
	srand(time(NULL));
}

KafkaClient::~KafkaClient()
{
}

int KafkaClient::Init()
{
	std::string broker_list = "w-w1901.add.nbt.qihoo.net:9092,w-w1902.add.nbt.qihoo.net:9092,w-w1903.add.nbt.qihoo.net:9092";

	network_ = new Network();
	network_->Init(this, broker_list);

	std::cout << "KafkaClient init OK!" << std::endl;
}

int KafkaClient::Start()
{
	network_->Start();

	//GroupCoordinatorRequest *group_request = new GroupCoordinatorRequest(0, "group");
	//auto it = nodes_.begin();
	//std::advance(it, rand() % nodes_.size());
	//Node *node = it->second;
	//PushRequest(node, group_request);

	std::vector<std::string> topic({"test"});
	JoinGroupRequest *join_request = new JoinGroupRequest(1, "group", "", topic);
	Node *node = nodes_["w-w1902.add.nbt.qihoo.net"];
	PushRequest(node, join_request);

	Response *response;
	short api_key = PopResponse(node, &response);

	switch(api_key)
	{
		case 10:
		{
			GroupCoordinatorResponse *coor_response = dynamic_cast<GroupCoordinatorResponse*>(response);

			std::cout << "api key = " << coor_response->api_key_ << std::endl;
			std::cout << "correlation id = " << coor_response->correlation_id_ << std::endl;
			std::cout << "error code = " << coor_response->error_code_ << std::endl;
			std::cout << "coordinator id = " << coor_response->coordinator_id_ << std::endl;
			std::cout << "coordinator host = " << coor_response->coordinator_host_ << std::endl;
			std::cout << "coordinator port = " << coor_response->coordinator_port_ << std::endl;
			std::cout << "total size = " << coor_response->total_size_ << std::endl;

			break;
		}
		case 11:
		{
			JoinGroupResponse *join_response = dynamic_cast<JoinGroupResponse*>(response);

			std::cout << "api key = " << join_response->api_key_ << std::endl;
			std::cout << "correlation id = " << join_response->correlation_id_ << std::endl;
			std::cout << "error code = " << join_response->error_code_ << std::endl;
			std::cout << "generation id = " << join_response->generation_id_ << std::endl;
			std::cout << "group protocol = " << join_response->group_protocol_ << std::endl;
			std::cout << "leader id = " << join_response->leader_id_ << std::endl;
			std::cout << "member id = " << join_response->member_id_ << std::endl;
			std::cout << "members:" << std::endl;
			for (int i = 0; i < join_response->members_.size(); i++)
			{
				Member &member = join_response->members_[i];

				std::cout << "	member id = " << member.member_id_ << std::endl;
				std::cout << "	member metadata = " << member.member_metadata_ << std::endl;
			}
			std::cout << "total size = " << join_response->total_size_ << std::endl;

			break;
		}
	}

	sleep(3);
	network_->Stop();
}

int KafkaClient::PushRequest(Node *node, Request *request)
{
	int fd = node->fd_;
	network_->send_queues_[fd].Push(request);
}

short KafkaClient::PopResponse(Node *node, Response **response)
{
	int fd = node->fd_;

	// -1: wait forever
	Response *r = network_->receive_queues_[fd].Pop(-1);
	*response = r;

	return r->api_key_;
}

