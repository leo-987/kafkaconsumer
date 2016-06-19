#include <string>

#include "kafka_client.h"

int main()
{
	{
		std::string brokers = "10.123.81.11:9092,10.123.81.12:9092,10.123.81.13:9092";
		std::string topic = "test";
		std::string group = "group";
		KafkaClient client(brokers, topic, group);
		client.Start();
	}

    return 0;
}

// TODO:merge Broker and Node in network.cc
