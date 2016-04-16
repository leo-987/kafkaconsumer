#include "metadata_response.h"
#include "util.h"
#include "easylogging++.h"
#include "error_code.h"


PartitionMetadata::PartitionMetadata(char **buf)
{
	partition_error_code_ = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	LOG_IF(partition_error_code_ != 0, ERROR) << "partition error code = " << partition_error_code_;

	partition_id_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;

	leader_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;

	int array_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int k = 0; k < array_size; k++)
	{
		int rep = Util::NetBytesToInt(*buf);
		(*buf) += 4;
		replicas_.push_back(rep);
	}

	array_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int k = 0; k < array_size; k++)
	{
		isr_.push_back(Util::NetBytesToInt(*buf));
		(*buf) += 4;
	}
}

int PartitionMetadata::CountSize()
{
	int size = 0;
	size += 2 + 4 + 4;
	size += 4 + 4 * replicas_.size();
	size += 4 + 4 * isr_.size();
	return size;
}

void PartitionMetadata::PrintAll()
{
	LOG(DEBUG) << "partition error code = " << partition_error_code_;
	LOG(DEBUG) << "partition id = " << partition_id_;
	LOG(DEBUG) << "leader = " << leader_;

	for (auto it = replicas_.begin(); it != replicas_.end(); ++it)
		LOG(DEBUG) << "replicas = " << *it;

	for (auto it = isr_.begin(); it != isr_.end(); ++it)
		LOG(DEBUG) << "isr = " << *it;
}

//--------------------------------------------
TopicMetadata::TopicMetadata(char **buf)
{
	topic_error_code_ = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	LOG_IF(topic_error_code_ != 0, ERROR) << "error code = " << topic_error_code_;

	short topic_name_size = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	topic_ = std::string(*buf, topic_name_size);
	(*buf) += topic_name_size;

	int partition_metadata_array_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int j = 0; j < partition_metadata_array_size; j++)
	{
		std::shared_ptr<PartitionMetadata> partition_metadata = std::make_shared<PartitionMetadata>(buf);
		partition_metadata_.push_back(partition_metadata);
	}
}

int TopicMetadata::CountSize()
{
	int size = 0;
	size += 2 + 2 + topic_.length();

	size += 4;
	for (auto it = partition_metadata_.begin(); it != partition_metadata_.end(); ++it)
		size += (*it)->CountSize();

	return size;
}

void TopicMetadata::PrintAll()
{
	LOG(DEBUG) << "topic error code = " << topic_error_code_;
	LOG(DEBUG) << "topic name = " << topic_;

	for (auto it = partition_metadata_.begin(); it != partition_metadata_.end(); ++it)
		(*it)->PrintAll();
}

//-------------------------------------------------
MetadataResponse::MetadataResponse(char **buf)
	: Response(ApiKey::MetadataType, buf)
{
	int array_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int i = 0; i < array_size; i++)
	{
		std::shared_ptr<Broker> broker = std::make_shared<Broker>(buf);
		brokers_.push_back(broker);
	}

	int topic_metadata_array_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int i = 0; i < topic_metadata_array_size; i++)
	{
		std::shared_ptr<TopicMetadata> topic_metadata = std::make_shared<TopicMetadata>(buf);
		topic_metadata_.push_back(topic_metadata);
	}

	if (Response::GetTotalSize() != CountSize())
	{
		LOG(ERROR) << "CountSize are not equal";
		throw 1;
	}
}

int MetadataResponse::CountSize()
{
	int size = Response::CountSize();

	// array
	size += 4;
	for (auto b_it = brokers_.begin(); b_it != brokers_.end(); ++b_it)
		size += (*b_it)->CountSize();

	// array
	size += 4;
	for (auto t_it = topic_metadata_.begin(); t_it != topic_metadata_.end(); ++t_it)
		size += (*t_it)->CountSize();

	return size;
}

void MetadataResponse::PrintAll()
{
	LOG(DEBUG) << "-----MetadataResponse-----";
	Response::PrintAll();
	for (auto b_it = brokers_.begin(); b_it != brokers_.end(); ++b_it)
		(*b_it)->PrintAll();
	for (auto t_it = topic_metadata_.begin(); t_it != topic_metadata_.end(); ++t_it)
		(*t_it)->PrintAll();
	LOG(DEBUG) << "--------------------------";
}

int MetadataResponse::GetFdFromIp(const std::string &alive_ip, const std::unordered_map<int, Broker> &origin_brokers)
{
	for (auto b_it = origin_brokers.begin(); b_it != origin_brokers.end(); ++b_it)
	{
		const Broker &b = b_it->second;
		if (b.ip_ == alive_ip)
			return b.fd_;
	}
	return -1;
}

// XXX: we should parse all broker data in response
void MetadataResponse::ParseBrokers(std::unordered_map<int, Broker> &updated_brokers)
{
	for (auto b_it = brokers_.begin(); b_it != brokers_.end(); ++b_it)
	{
		//Broker alive_broker(-1, b_it->id_, b_it->ip_, b_it->port_);
		updated_brokers.insert({(*b_it)->id_, **b_it});
	}
}

// Parse the partitions of a topic, skip all invalid partitions
// return value:
// 1. topic error code
// 2. no error
int16_t MetadataResponse::ParsePartitions(std::unordered_map<int, Partition> &partitions)
{
	partitions.clear();

	// XXX: assuming only one topic
	TopicMetadata &tm = *(topic_metadata_[0]);
	int16_t topic_error_code = tm.topic_error_code_;
	if (topic_error_code != ErrorCode::NO_ERROR)
	{
		// 1. UnknownTopicOrPartition(3)
		// 2. InvalidTopic(17)
		// 3. TopicAuthorizationFailed(29)
		return topic_error_code;
	}

	std::vector<std::shared_ptr<PartitionMetadata>> &pm = tm.partition_metadata_;
	for (auto pm_it = pm.begin(); pm_it != pm.end(); ++pm_it)
	{
		if ((*pm_it)->partition_error_code_ != ErrorCode::NO_ERROR)
		{
			// 1. UnknownTopicOrPartition(3)
			// 2. LeaderNotAvailable(5)
			continue;
		}

		Partition partition((*pm_it)->partition_id_, (*pm_it)->leader_);
		partitions[partition.GetPartitionId()] = partition;
	}
	return ErrorCode::NO_ERROR;
}


