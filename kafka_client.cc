#include <iostream>
#include <unistd.h>

#include "kafka_client.h"


KafkaClient::KafkaClient()
{
}

KafkaClient::~KafkaClient()
{
}

int KafkaClient::Init()
{
	network_ = new Network();
	network_->Init(this);

	std::cout << "KafkaClient init OK!" << std::endl;
}

int KafkaClient::Start()
{
	network_->Start();
	GroupCoordinatorRequest *group_request = new GroupCoordinatorRequest(0, "group");
	send_queue_.Push(group_request);
	sleep(3);
	network_->Stop();
}




