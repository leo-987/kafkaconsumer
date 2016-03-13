#ifndef _UTIL_H_
#define _UTIL_H_

#if 0
#ifdef __APPLE__
#include <machine/endian.h>
#include <libkern/OSByteOrder.h>

#define htobe16(x) OSSwapHostToBigInt16(x)
#define htole16(x) OSSwapHostToLittleInt16(x)
#define be16toh(x) OSSwapBigToHostInt16(x)
#define le16toh(x) OSSwapLittleToHostInt16(x)

#define htobe32(x) OSSwapHostToBigInt32(x)
#define htole32(x) OSSwapHostToLittleInt32(x)
#define be32toh(x) OSSwapBigToHostInt32(x)
#define le32toh(x) OSSwapLittleToHostInt32(x)

#define htobe64(x) OSSwapHostToBigInt64(x)
#define htole64(x) OSSwapHostToLittleInt64(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#define le64toh(x) OSSwapLittleToHostInt64(x)

#define __BIG_ENDIAN    BIG_ENDIAN
#define __LITTLE_ENDIAN LITTLE_ENDIAN
#define __BYTE_ORDER    BYTE_ORDER
#endif
#endif


namespace Util
{
	using namespace std;

	// trim from start
	inline string &Ltrim(string &s);

	// trim from end
	inline string &Rtrim(string &s);

	// trim from both ends
	inline string &Trim(string &s);

	vector<string> Split(const string &str, char delimiter);

	int NetBytesToInt(char *buf);

	short NetBytesToShort(char *buf);

	string HostnameToIp(const string &hostname);
} // end of namespace

#endif
