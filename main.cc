
#include "kafka_client.h"

enum class State {
	down,
	startup,
	joined,
};


int main()
{
	KafkaClient client;
	client.Init();
	client.Start();

    return 0;
}
