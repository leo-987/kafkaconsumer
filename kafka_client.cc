#include <unistd.h>

#include "kafka_client.h"


KafkaClient::KafkaClient()
{
	network_ = new Network();
	network_->Init(this);
	network_->Start();
	sleep(2);
	network_->Stop();
}


KafkaClient::~KafkaClient()
{
}






