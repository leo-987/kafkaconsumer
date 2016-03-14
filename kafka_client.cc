#include <iostream>
#include <unistd.h>

#include "kafka_client.h"

bool KafkaClient::run_;

void KafkaClient::SignalHandler(int signal)
{
	std::cout << "Interrupt!" << std::endl;
	run_ = false;
}

KafkaClient::KafkaClient(const std::string &brokers, const std::string &topic, const std::string &group)
{
	srand(time(NULL));
	network_ = new Network(this, brokers, topic, group);
	//signal(SIGINT, KafkaClient::SignalHandler);
	std::cout << "KafkaClient init OK!" << std::endl;
}

KafkaClient::~KafkaClient()
{
	//delete state_machine_;
	//delete network_;
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


