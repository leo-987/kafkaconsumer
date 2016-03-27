#include "join_group_request.h"
#include "easylogging++.h"

ProtocolMetadata::ProtocolMetadata(const std::vector<std::string> &topics)
{
	version_ = 0;
	subscription_ = topics;
	user_data_ = "";
}

int ProtocolMetadata::CountSize()
{
	int size = 0;
	size += 2;		// version
	size += 4;		// array
	for (auto s_it = subscription_.begin(); s_it != subscription_.end(); ++s_it)
		size += 2 + s_it->size();
	size += 4 + user_data_.size();
	return size;
}

void ProtocolMetadata::Package(char **buf)
{
	// version
	short version = htons(version_);
	memcpy(*buf, &version, 2);
	(*buf) += 2;

	// topics
	int topics_size = htonl(subscription_.size());
	memcpy(*buf, &topics_size, 4);
	(*buf) += 4;

	for (auto sub_it = subscription_.begin(); sub_it != subscription_.end(); ++sub_it)
	{
		std::string &topic = *sub_it;
		short topic_size = htons((short)topic.length());
		memcpy(*buf, &topic_size, 2);
		(*buf) += 2;
		memcpy(*buf, topic.c_str(), topic.length());
		(*buf) += topic.length();
	}

	// user data
	int user_data_size = htonl(user_data_.size());
	memcpy(*buf, &user_data_size, 4);
	(*buf) += 4;
	memcpy(*buf, user_data_.data(), user_data_.size());
}

//---------------------------------------------
GroupProtocol::GroupProtocol(const std::vector<std::string> &topics)
	: protocol_metadata_(topics)
{
	assignment_strategy_ = "range";
}

int GroupProtocol::CountSize()
{
	return 2 + assignment_strategy_.length() +
		   4 + protocol_metadata_.CountSize();
}

void GroupProtocol::Package(char **buf)
{
	// assignment strategy
	short assignment_strategy_size = htons((short)assignment_strategy_.length());
	memcpy(*buf, &assignment_strategy_size, 2);
	(*buf) += 2;
	memcpy(*buf, assignment_strategy_.c_str(), assignment_strategy_.length());
	(*buf) += assignment_strategy_.length();

	// ProtocolMetadata bytes size
	int protocol_metadata_size = htonl(protocol_metadata_.CountSize());
	memcpy(*buf, &protocol_metadata_size, 4);
	(*buf) += 4;

	protocol_metadata_.Package(buf);
}

//----------------------------------------------
JoinGroupRequest::JoinGroupRequest(const std::string &group_id, const std::string &member_id,
			const std::vector<std::string> &topics, int correlation_id, int32_t session_timeout)
	: Request(ApiKey::JoinGroupType, correlation_id)
{
	group_id_ = group_id;
	session_timeout_ = session_timeout;
	member_id_ = member_id;
	protocol_type_ = "consumer";

	// we assume only one group protocol
	GroupProtocol group_protocol(topics);
	group_protocols_.push_back(group_protocol);
	total_size_ = CountSize();
}

int JoinGroupRequest::CountSize()
{
	int size = Request::CountSize();
	size +=	2 + group_id_.length() + 4 + 2 + member_id_.length() + 2 + protocol_type_.length();
	size += 4;
	for (auto gp_it = group_protocols_.begin(); gp_it != group_protocols_.end(); ++gp_it)
		size += gp_it->CountSize();
	return size;
}

void JoinGroupRequest::PrintAll()
{
	LOG(DEBUG) << "-----JoinGroupRequest-----";
	Request::PrintAll();
	LOG(DEBUG) << "group id = " << group_id_;
	LOG(DEBUG) << "session timeout = " << session_timeout_;
	LOG(DEBUG) << "member id = " << member_id_;
	LOG(DEBUG) << "protocol type = " << protocol_type_;

	for (unsigned int i = 0; i < group_protocols_.size(); i++)
	{
		GroupProtocol &gp = group_protocols_[i];
		LOG(DEBUG) << "	assignment strategy = " << gp.assignment_strategy_;
		LOG(DEBUG) << "	version = " << gp.protocol_metadata_.version_;
		for (unsigned int i = 0; i < gp.protocol_metadata_.subscription_.size(); i++)
			LOG(DEBUG) << "	subscription = " << gp.protocol_metadata_.subscription_[i];
		LOG(DEBUG) << "	user data = " << gp.protocol_metadata_.user_data_;
	}
	LOG(DEBUG) << "--------------------------";
}

void JoinGroupRequest::Package(char **buf)
{
	Request::Package(buf);

	// group id
	short group_id_size = htons((short)group_id_.length());
	memcpy(*buf, &group_id_size, 2);
	(*buf) += 2;
	memcpy(*buf, group_id_.c_str(), group_id_.length());
	(*buf) += group_id_.length();

	// session timeout
	int session_timeout = htonl(session_timeout_);
	memcpy(*buf, &session_timeout, 4);
	(*buf) += 4;

	// member id
	short member_id_size = htons((short)member_id_.length());
	memcpy(*buf, &member_id_size, 2);
	(*buf) += 2;
	memcpy(*buf, member_id_.c_str(), member_id_.length());
	(*buf) += member_id_.length();

	// protocol type
	short protocol_type_size = htons((short)protocol_type_.length());
	memcpy(*buf, &protocol_type_size, 2);
	(*buf) += 2;
	memcpy(*buf, protocol_type_.c_str(), protocol_type_.length());
	(*buf) += protocol_type_.length();

	// array size
	int group_protocols_size = htonl(group_protocols_.size());
	memcpy(*buf, &group_protocols_size, 4);
	(*buf) += 4;

	for (auto gp_it = group_protocols_.begin(); gp_it != group_protocols_.end(); ++gp_it)
	{
		gp_it->Package(buf);
	}
}


