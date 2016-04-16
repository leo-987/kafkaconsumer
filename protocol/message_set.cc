#include <string.h>
#include <iostream>

#include "message_set.h"
#include "util.h"
#include "easylogging++.h"

//Message::Message()
//{
//}

Message::Message(char **buf)
{
	crc_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;

	memcpy(&magic_byte_, *buf, 1);
	(*buf) += 1;
	memcpy(&attributes_, *buf, 1);
	(*buf) += 1;

	int key_len = Util::NetBytesToInt(*buf);
	key_len = (key_len == -1 ? 0 : key_len);
	(*buf) += 4;
	key_ = std::string(*buf, key_len);
	(*buf) += key_len;

	int value_len = Util::NetBytesToInt(*buf);
	value_len = (value_len == -1 ? 0 : value_len);
	(*buf) += 4;
	value_ = std::string(*buf, value_len);
	(*buf) += value_len;
}

int Message::CountSize()
{
	return 4 + 1 + 1 + 4 + key_.length() + 4 + value_.length();
}

void Message::PrintAll()
{
	LOG(DEBUG) << "crc = " << crc_;
	LOG(DEBUG) << "magic byte = " << magic_byte_;
	LOG(DEBUG) << "attributes = " << attributes_;
	LOG(DEBUG) << "key = " << key_;
	LOG(DEBUG) << "value = " << value_;
}

OffsetAndMessage::OffsetAndMessage()
{
}

OffsetAndMessage::OffsetAndMessage(char **buf, int message_set_size, int &sum, int &invalid_bytes)
{
	long offset;
	memcpy(&offset, *buf, 8);
	// for Linux
	offset_ = be64toh(offset);
	// for Mac
	//offset_ = ntohll(offset);
	(*buf) += 8;
	sum +=8;
	if (sum >= message_set_size)
	{
		invalid_bytes += message_set_size - (sum - 8);
		throw 1;
	}

	message_size_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;
	sum +=4;
	if (sum > message_set_size || sum + message_size_ > message_set_size)
	{
		invalid_bytes += message_set_size - (sum - 12);
		throw 1;
	}

	//message_ = Message(buf);
	message_ = std::make_shared<Message>(buf);
	sum += message_->CountSize();
}

int OffsetAndMessage::CountSize()
{
	return 8 + 4 + message_->CountSize();
}

void OffsetAndMessage::PrintAll()
{
	LOG(DEBUG) << "offset = " << offset_;
	LOG(DEBUG) << "message size = " << message_size_;
	message_->PrintAll();
}

MessageSet::MessageSet()
{
}

MessageSet::MessageSet(char **buf, int message_set_size, int &invalid_bytes)
{
	int sum = 0;
	while (sum != message_set_size)
	{
		try
		{
			OffsetAndMessage offset_message(buf, message_set_size, sum, invalid_bytes);
			offset_message_.push_back(offset_message);
		}
		catch (...)
		{
			(*buf) += message_set_size - sum;
			break;
		}
	}
}

int MessageSet::CountSize()
{
	// NOTE: not 4
	int size = 0;
	for (auto om_it = offset_message_.begin(); om_it != offset_message_.end(); ++om_it)
		size += om_it->CountSize();
	return size;
}

void MessageSet::PrintAll()
{
	for (auto om_it = offset_message_.begin(); om_it != offset_message_.end(); ++om_it)
		om_it->PrintAll();
}

void MessageSet::PrintMsg()
{
	for (auto om_it = offset_message_.begin(); om_it != offset_message_.end(); ++om_it)
	{
		Message &msg = *(om_it->message_);
		//std::cout << "key: " << msg.key_ << "	value: " << msg.value_ << std::endl;
		//std::cout << "offset: " << om_it->offset_ << "	value: " << msg.value_ << std::endl;
		std::cout << msg.value_ << std::endl;
	}
}

int64_t MessageSet::GetLastOffset()
{
	// Only get the last msg's offset
	int size = offset_message_.size();
	return offset_message_[size - 1].offset_;
}


