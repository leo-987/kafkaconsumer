#ifndef _ERROR_CODE_H_
#define _ERROR_CODE_H_

// Api key is short, so use struct rather than enum
struct ErrorCode {
	static const int16_t NO_ERROR             = 0;
	static const int16_t ILLEGAL_GENERATION   = 22;
	static const int16_t UNKNOWN_MEMBER_ID    = 25;
};

#endif
