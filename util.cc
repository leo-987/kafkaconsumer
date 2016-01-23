#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <arpa/inet.h>

#include "util.h"

namespace Util
{
using namespace std;

// trim from start
inline string &ltrim(string &s)
{
	s.erase(s.begin(), find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace))));
	return s;
}


// trim from end
inline string &rtrim(string &s)
{
	s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(), s.end());
	return s;
}


// trim from both ends
inline string &trim(string &s)
{
	return ltrim(rtrim(s));
}


vector<string> split(const string &str, char delimiter)
{
	vector<string> internal;
	stringstream ss(str);
	string tok;

	while(getline(ss, tok, delimiter))
	{
		internal.push_back(trim(tok));
	}

	return internal;
}


int net_bytes_to_int(char *buf)
{
	int a = buf[0] & 0xFF;
	a |= ((buf[1] << 8) & 0xFF00);
	a |= ((buf[2] << 16) & 0xFF0000);
	a |= ((buf[3] << 24) & 0xFF000000);
	return ntohl(a);
}


short net_bytes_to_short(char *buf)
{
	int a = buf[0] & 0xFF;
	a |= ((buf[1] << 8) & 0xFF00);
	return ntohs(a);
}

} // end of namespace

