#include <iostream>

#include "metadata_response.h"
#include "util.h"

#if 0
Broker::Broker(int fd, int node_id, const std::string &host, int port)
{
	node_id_ = node_id;
	host_ = host;
	port_ = port;
}

Broker::Broker(char **buf)
{
	// node id
	node_id_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;

	// host name
	short host_size = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	host_ = std::string(*buf, host_size);
	(*buf) += host_size;

	// port
	port_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;
}

int Broker::CountSize()
{
	return 4 + 2 + host_.length() + 4;
}

void Broker::PrintAll()
{
	std::cout << "node id = " << node_id_ << std::endl;
	std::cout << "host = " << host_ << std::endl;
	std::cout << "port = " << port_ << std::endl;
}
#endif

PartitionMetadata::PartitionMetadata(short error_code, int partition_id, int leader,
		const std::vector<int> &replicas, const std::vector<int> &isr)
{
	partition_error_code_ = error_code;
	partition_id_ = partition_id;
	leader_ = leader;
	replicas_ = replicas;
	isr_ = isr;
}

PartitionMetadata::PartitionMetadata(char **buf)
{
	// partition error code
	partition_error_code_ = Util::NetBytesToShort(*buf);
	(*buf) += 2;

	// partition id
	partition_id_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;

	// leader
	leader_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;

	// replicas array size
	int replicas_array_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int k = 0; k < replicas_array_size; k++)
	{
		int rep = Util::NetBytesToInt(*buf);
		(*buf) += 4;
		replicas_.push_back(rep);
	}

	// isr array size
	int isr_array_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int k = 0; k < isr_array_size; k++)
	{
		isr_.push_back(Util::NetBytesToInt(*buf));
		(*buf) += 4;
	}
}

int PartitionMetadata::CountSize()
{
	int size = 0;
	size += 2 + 4 + 4;

	// array
	size += 4 + 4 * replicas_.size();

	// array
	size += 4 + 4 * isr_.size();

	return size;
}

void PartitionMetadata::PrintAll()
{
	std::cout << "partition error code = " << partition_error_code_ << std::endl;
	std::cout << "partition id = " << partition_id_ << std::endl;
	std::cout << "leader = " << leader_ << std::endl;

	for (auto it = replicas_.begin(); it != replicas_.end(); ++it)
		std::cout << "replicas = " << *it << std::endl;

	for (auto it = isr_.begin(); it != isr_.end(); ++it)
		std::cout << "isr = " << *it << std::endl;
}

TopicMetadata::TopicMetadata(short error_code, const std::string &topic_name,
		const std::vector<PartitionMetadata> &partition_metadata)
{
	topic_error_code_ = error_code;
	topic_ = topic_name;
	partition_metadata_ = partition_metadata;
}

TopicMetadata::TopicMetadata(char **buf)
{
	// topic error code
	topic_error_code_ = Util::NetBytesToShort(*buf);
	(*buf) += 2;

	// topic name
	short topic_name_size = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	topic_ = std::string(*buf, topic_name_size);
	(*buf) += topic_name_size;

	// partition metadata array
	int partition_metadata_array_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int j = 0; j < partition_metadata_array_size; j++)
	{
		PartitionMetadata partition_metadata(buf);
		partition_metadata_.push_back(partition_metadata);
	}
}

int TopicMetadata::CountSize()
{
	int size = 0;
	size += 2 + 2 + topic_.length();

	size += 4;
	for (auto it = partition_metadata_.begin(); it != partition_metadata_.end(); ++it)
		size += it->CountSize();

	return size;
}

void TopicMetadata::PrintAll()
{
	std::cout << "topic error code = " << topic_error_code_ << std::endl;
	std::cout << "topic name = " << topic_ << std::endl;

	for (auto it = partition_metadata_.begin(); it != partition_metadata_.end(); ++it)
		it->PrintAll();
}

MetadataResponse::MetadataResponse(char **buf)
	: Response(ApiKey::MetadataType, buf)
{
	// broker array
	int broker_array_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int i = 0; i < broker_array_size; i++)
	{
		Broker broker(buf);
		brokers_.push_back(broker);
	}

	// topic metadata array
	int topic_metadata_array_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int i = 0; i < topic_metadata_array_size; i++)
	{
		TopicMetadata topic_metadata(buf);
		topic_metadata_.push_back(topic_metadata);
	}

	if (total_size_ != CountSize())
	{
		throw "CountSize are not equal";
	}
}

int MetadataResponse::CountSize()
{
	int size = Response::CountSize();

	// array
	size += 4;
	for (auto b_it = brokers_.begin(); b_it != brokers_.end(); ++b_it)
		size += b_it->CountSize();

	// array
	size += 4;
	for (auto t_it = topic_metadata_.begin(); t_it != topic_metadata_.end(); ++t_it)
		size += t_it->CountSize();

	return size;
}

void MetadataResponse::PrintAll()
{
	std::cout << "-----MetadataResponse-----" << std::endl;
	Response::PrintAll();
	for (auto b_it = brokers_.begin(); b_it != brokers_.end(); ++b_it)
		b_it->PrintAll();

	for (auto t_it = topic_metadata_.begin(); t_it != topic_metadata_.end(); ++t_it)
		t_it->PrintAll();
	std::cout << "--------------------------" << std::endl;
}

int MetadataResponse::GetBrokerIdFromHostname(const std::string &hostname)
{
	for (auto b_it = brokers_.begin(); b_it != brokers_.end(); ++b_it)
	{
		if (b_it->host_ == hostname)
			return b_it->id_;
	}
	return -1;
}

// XXX: we should parse all broker data in response
void MetadataResponse::ParseBrokers(std::map<int, Broker> &brokers)
{
	// insert new broker
	for (auto b_it = brokers.begin(); b_it != brokers.end(); ++b_it)
	{
		Broker b = b_it->second;
		int broker_id = GetBrokerIdFromHostname(b.host_);
		if (broker_id < 0)
		{
			std::cout << "error: broker id not found" << std::endl;
			continue;
		}

		b.id_ = broker_id;
		brokers.insert({broker_id, b});
	}

	// delete tmp broker
	for (auto b_it = brokers.begin(); b_it != brokers.end(); /* NULL */)
	{
		if (b_it->first < 0)
			b_it = brokers.erase(b_it);
		else
			++b_it;
	}
}

void MetadataResponse::ParsePartitions(std::map<int, Partition> &partitions)
{
	// XXX: assuming only one topic
	std::vector<PartitionMetadata> &pm = topic_metadata_[0].partition_metadata_;
	for (auto pm_it = pm.begin(); pm_it != pm.end(); ++pm_it)
	{
		Partition partition(pm_it->partition_id_, pm_it->leader_);
		partitions.insert({partition.GetPartitionId(), partition});
	}
}



