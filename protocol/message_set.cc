#include <string.h>
#include <iostream>

#include "message_set.h"
#include "util.h"

Message::Message()
{
}

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
	std::cout << "crc = " << crc_ << std::endl;
	std::cout << "magic byte = " << magic_byte_ << std::endl;
	std::cout << "attributes = " << attributes_ << std::endl;
	std::cout << "key = " << key_ << std::endl;
	std::cout << "value = " << value_ << std::endl;
}

OffsetAndMessage::OffsetAndMessage()
{
}

OffsetAndMessage::OffsetAndMessage(char **buf)
{
	long offset;
	memcpy(&offset, *buf, 8);
	// for Linux
	//offset_ = be64toh(offset);
	// for Mac
	offset_ = ntohll(offset);
	(*buf) += 8;

	message_size_ = Util::NetBytesToInt(*buf);
	(*buf) += 4;

	message_ = Message(buf);
}

int OffsetAndMessage::CountSize()
{
	return 8 + 4 + message_.CountSize();
}

void OffsetAndMessage::PrintAll()
{
	std::cout << "offset = " << offset_ << std::endl;
	std::cout << "message size = " << message_size_ << std::endl;
	message_.PrintAll();
}

MessageSet::MessageSet()
{
}

MessageSet::MessageSet(char **buf, int message_set_size)
{
	int sum = 0;
	while (sum != message_set_size)
	{
		OffsetAndMessage offset_message(buf);
		offset_message_.push_back(offset_message);
		sum += offset_message.CountSize();
	}
}

int MessageSet::CountSize()
{
	int size = 0;
	for (auto om_it = offset_message_.begin(); om_it != offset_message_.end(); ++om_it)
	{
		size += om_it->CountSize();
	}
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
		Message &msg = om_it->message_;
		std::cout << "key: " << msg.key_ << std::endl;
		std::cout << "value: " << msg.value_ << std::endl;
	}
}

int64_t MessageSet::GetLastOffset()
{
	// Only get the last msg's offset
	int size = offset_message_.size();
	return offset_message_[size - 1].offset_;
}



