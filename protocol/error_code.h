#ifndef _ERROR_CODE_H_
#define _ERROR_CODE_H_

// Api key is short, so use struct rather than enum
struct ErrorCode {
	static const short ILLEGAL_GENERATION   = 22;
	static const short UNKNOWN_MEMBER_ID    = 25;
};

#endif
