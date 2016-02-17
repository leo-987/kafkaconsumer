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
	std::string broker_list = "w-w1901.add.nbt.qihoo.net:9092,\
							   w-w1902.add.nbt.qihoo.net:9092,w-w1903.add.nbt.qihoo.net:9092";


	network_ = new Network();
	network_->Init(this, broker_list);

	state_machine_ = new StateMachine(network_, nodes_);
	state_machine_->Init();

	std::cout << "KafkaClient init OK!" << std::endl;

	return 0;
}

int KafkaClient::Start()
{
	network_->Start();
	state_machine_->Start();

	state_machine_->Stop();
	network_->Stop();

	return 0;
}

#if 0
int KafkaClient::PushRequest(Node *node, Request *request)
{
	int fd = node->fd_;
	network_->send_queues_[fd].Push(request);
	return 0;
}

short KafkaClient::PopResponse(Node *node, Response **response)
{
	int fd = node->fd_;

	// -1: wait forever
	Response *r = network_->receive_queues_[fd].Pop(-1);
	*response = r;

	return r->api_key_;
}
#endif
