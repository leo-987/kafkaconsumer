#include "metadata_response.h"
#include "util.h"
#include "easylogging++.h"

//PartitionMetadata::PartitionMetadata(short error_code, int partition_id, int leader,
//		const std::vector<int> &replicas, const std::vector<int> &isr)
//{
//	partition_error_code_ = error_code;
//	LOG_IF(partition_error_code_ != 0, ERROR) << "error code = " << partition_error_code_;
//	partition_id_ = partition_id;
//	leader_ = leader;
//	replicas_ = replicas;
//	isr_ = isr;
//}

PartitionMetadata::PartitionMetadata(char **buf)
{
	partition_error_code_ = Util::NetBytesToShort(*buf);
	(*buf) += 2;
	LOG_IF(partition_error_code_ != 0, ERROR) << "error code = " << partition_error_code_;

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
//TopicMetadata::TopicMetadata(short error_code, const std::string &topic_name,
//		const std::vector<PartitionMetadata> &partition_metadata)
//{
//	topic_error_code_ = error_code;
//	topic_ = topic_name;
//	partition_metadata_ = partition_metadata;
//}

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
	LOG(DEBUG) << "topic error code = " << topic_error_code_;
	LOG(DEBUG) << "topic name = " << topic_;

	for (auto it = partition_metadata_.begin(); it != partition_metadata_.end(); ++it)
		it->PrintAll();
}

//-------------------------------------------------
MetadataResponse::MetadataResponse(char **buf)
	: Response(ApiKey::MetadataType, buf)
{
	int array_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int i = 0; i < array_size; i++)
	{
		Broker broker(buf);
		brokers_.push_back(broker);
	}

	int topic_metadata_array_size = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	for (int i = 0; i < topic_metadata_array_size; i++)
	{
		TopicMetadata topic_metadata(buf);
		topic_metadata_.push_back(topic_metadata);
	}

	if (Response::GetTotalSize() != CountSize())
	{
		LOG(ERROR) << "CountSize are not equal";
		throw;
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
	LOG(DEBUG) << "-----MetadataResponse-----";
	Response::PrintAll();
	for (auto b_it = brokers_.begin(); b_it != brokers_.end(); ++b_it)
		b_it->PrintAll();
	for (auto t_it = topic_metadata_.begin(); t_it != topic_metadata_.end(); ++t_it)
		t_it->PrintAll();
	LOG(DEBUG) << "--------------------------";
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
std::unordered_map<int, Broker> MetadataResponse::ParseBrokers(const std::unordered_map<int, Broker> &brokers)
{
	std::unordered_map<int, Broker> updated_brokers;

	// insert new broker
	for (auto b_it = brokers.begin(); b_it != brokers.end(); ++b_it)
	{
		Broker b = b_it->second;
		int broker_id = GetBrokerIdFromHostname(b.host_);
		if (broker_id < 0)
		{
			LOG(ERROR) << "error: broker id not found";
			continue;
		}

		b.id_ = broker_id;
		//brokers.insert({broker_id, b});
		updated_brokers.insert({broker_id, b});
	}

	// delete tmp broker
	//for (auto b_it = brokers.begin(); b_it != brokers.end(); /* NULL */)
	//{
	//	if (b_it->first < 0)
	//	{
	//		// for c++11
	//		//b_it = brokers.erase(b_it);

	//		auto to_erase = b_it;
	//		++b_it;
	//		brokers.erase(to_erase);
	//	}
	//	else
	//		++b_it;
	//}

	return updated_brokers;
}

int16_t MetadataResponse::ParsePartitions(std::unordered_map<int, Partition> &partitions)
{
	// XXX: assuming only one topic
	TopicMetadata &tm = topic_metadata_[0];
	int16_t topic_error_code = tm.topic_error_code_;
	if (topic_error_code != 0)
	{
		// 1. UnknownTopicOrPartition(3)
		// 2. InvalidTopic(17)
		// 3. TopicAuthorizationFailed(29)
		LOG(ERROR) << "topic error code = " << topic_error_code;
		return topic_error_code;
	}

	std::vector<PartitionMetadata> &pm = tm.partition_metadata_;
	for (auto pm_it = pm.begin(); pm_it != pm.end(); ++pm_it)
	{
		// 1. UnknownTopicOrPartition(3)
		// 2. LeaderNotAvailable(5)
		if (pm_it->partition_error_code_ != 0)
			continue;

		Partition partition(pm_it->partition_id_, pm_it->leader_);
		//partitions.insert({partition.GetPartitionId(), partition});
		partitions[partition.GetPartitionId()] = partition;
	}
	return 0;
}


