#ifndef _SYNC_GROUP_REQUEST_H_
#define _SYNC_GROUP_REQUEST_H_

#include <string>
#include <vector>
#include <map>

#include "request.h"
#include "member_assignment.h"

class GroupAssignment {
public:
	GroupAssignment(const std::string &topic, const std::string &member_id,
		const std::vector<int> &partitions);

	int CountSize();
	void PrintAll();
	int Package(char **buf);

	std::string member_id_;
	MemberAssignment member_assignment_;	// bytes
};

class SyncGroupRequest: public Request {
public:
	SyncGroupRequest(int correlation_id, const std::string &topic, const std::string group_id,
			int generation_id, const std::string &member_id,
			const std::map<std::string, std::vector<int>> &member_partition_map);

	virtual int CountSize();
	virtual void PrintAll();
	virtual int Package(char **buf);

	std::string group_id_;
	int generation_id_;
	std::string member_id_;
	std::vector<GroupAssignment> group_assignment_;		// array
};

#endif
