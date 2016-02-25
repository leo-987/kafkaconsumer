#include <iostream>
#include <unistd.h>

#include "kafka_client.h"

bool KafkaClient::run_;

void KafkaClient::SignalHandler(int signal)
{
	std::cout << "Interrupt!" << std::endl;
	run_ = false;
}

KafkaClient::KafkaClient()
{
}

KafkaClient::~KafkaClient()
{
	//delete state_machine_;
	//delete network_;
}

int KafkaClient::Init()
{
	srand(time(NULL));

	std::string broker_list = "w-w1901.add.nbt.qihoo.net:9092,\
							   w-w1902.add.nbt.qihoo.net:9092,w-w1903.add.nbt.qihoo.net:9092";

	network_ = new Network();
	network_->Init(this, broker_list);

	//signal(SIGINT, KafkaClient::SignalHandler);
	
	std::cout << "KafkaClient init OK!" << std::endl;

	return 0;
}

int KafkaClient::Start()
{
	network_->Start();
	return 0;
}

int KafkaClient::Stop()
{
	//state_machine_->Stop();
	network_->Stop();

	return 0;
}


