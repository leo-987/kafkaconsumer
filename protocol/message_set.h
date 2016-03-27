#ifndef _MESSAGE_SETS_H_
#define _MESSAGE_SETS_H_

#include <stdint.h>
#include <string>
#include <vector>

class Message {
public:
	Message();
	Message(char **buf);

	int CountSize();
	void PrintAll();

	friend class MessageSet;
private:
	int crc_;
	int8_t magic_byte_;
	int8_t attributes_;
	std::string key_;	// bytes
	std::string value_;	// bytes
};

class OffsetAndMessage {
public:
	OffsetAndMessage();
	OffsetAndMessage(char **buf, int message_set_size, int &sum, int &invalid_bytes);

	int CountSize();
	void PrintAll();

	friend class MessageSet;
private:
	long offset_;
	int message_size_;	// the size of the subsequent request or response message in bytes
	Message message_;
};

class MessageSet {
public:
	MessageSet();
	MessageSet(char **buf, int message_set_size, int &invalid_bytes);

	int CountSize();
	void PrintAll();
	void PrintMsg();
	int64_t GetLastOffset();

private:
	std::vector<OffsetAndMessage> offset_message_;
};

#endif
