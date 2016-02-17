#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <iostream>

#include "util.h"

namespace Util
{
using namespace std;

// trim from start
inline string &Ltrim(string &s)
{
	s.erase(s.begin(), find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace))));
	return s;
}

// trim from end
inline string &Rtrim(string &s)
{
	s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(), s.end());
	return s;
}

// trim from both ends
inline string &Trim(string &s)
{
	return Ltrim(Rtrim(s));
}

vector<string> Split(const string &str, char delimiter)
{
	vector<string> internal;
	stringstream ss(str);
	string tok;

	while(getline(ss, tok, delimiter))
	{
		internal.push_back(Trim(tok));
	}

	return internal;
}

int NetBytesToInt(char *buf)
{
	int a = buf[0] & 0xFF;
	a |= ((buf[1] << 8) & 0xFF00);
	a |= ((buf[2] << 16) & 0xFF0000);
	a |= ((buf[3] << 24) & 0xFF000000);
	return ntohl(a);
}

short NetBytesToShort(char *buf)
{
	int a = buf[0] & 0xFF;
	a |= ((buf[1] << 8) & 0xFF00);
	return ntohs(a);
}

string HostnameToIp(const string &hostname)
{
	string ip = "";
	hostent *record = gethostbyname(hostname.c_str());
	if(record != NULL)
	{
		in_addr *address = (in_addr *)record->h_addr;
		ip = inet_ntoa(*address);
	}
	return ip;
}

} // end of namespace

