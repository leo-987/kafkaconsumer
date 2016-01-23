#ifndef _UTIL_H_
#define _UTIL_H_

namespace Util
{
	using namespace std;

	// trim from start
	inline string &ltrim(string &s);

	// trim from end
	inline string &rtrim(string &s);

	// trim from both ends
	inline string &trim(string &s);

	vector<string> split(const string &str, char delimiter);

	int net_bytes_to_int(char *buf);

	short net_bytes_to_short(char *buf);
} // end of namespace

#endif
