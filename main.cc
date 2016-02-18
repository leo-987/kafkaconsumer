
#include "kafka_client.h"

int main()
{
	{
		KafkaClient client;
		client.Init();
		client.Start();
	}

    return 0;
}
