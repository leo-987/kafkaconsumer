#ifndef _ERROR_CODE_H_
#define _ERROR_CODE_H_

// Api key is short, so use struct rather than enum
struct ErrorCode {
	static const int16_t NO_ERROR                        = 0;
	static const int16_t UNKNOWNTOPICORPARTITION         = 3;
	static const int16_t LEADERNOTAVAILABLE              = 5;
	static const int16_t GROUP_COORDINATOR_NOT_AVAILABLE = 15;
	static const int16_t INVALIDTOPIC                    = 17;
	static const int16_t ILLEGAL_GENERATION              = 22;
	static const int16_t UNKNOWN_MEMBER_ID               = 25;
	static const int16_t TOPICAUTHORIZATIONFAILED        = 29;
};

#endif
