#include <string>

#include "kafka_client.h"

int main()
{
	{
		std::string brokers = "w-w1901.add.nbt.qihoo.net:9092,\
							   w-w1902.add.nbt.qihoo.net:9092,w-w1903.add.nbt.qihoo.net:9092";
		std::string topic = "test";
		std::string group = "group";
		KafkaClient client(brokers, topic, group);
		client.Start();
	}

    return 0;
}

// TODO:merge Broker and Node in network.cc
