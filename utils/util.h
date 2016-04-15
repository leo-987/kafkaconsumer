#ifndef _UTIL_H_
#define _UTIL_H_

#include <vector>
#include <string>

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

	inline int ParseInt(char **buf)
	{
		int res = Util::NetBytesToInt(*buf);
		(*buf) += 4;
		return res;
	}

	inline int ParseShort(char **buf)
	{
		short res = Util::NetBytesToShort(*buf);
		(*buf) += 2;
		return res;
	}

	string HostnameToIp(const string &hostname);
} // end of namespace

#endif
