#ifndef _MEMBER_ASSIGNMENT_H_
#define _MEMBER_ASSIGNMENT_H_

#include <string>
#include <vector>

class PartitionAssignment {
public:
	PartitionAssignment(const std::string &topic, const std::vector<int> &partitions);
	PartitionAssignment(char **buf);

	int CountSize();
	void PrintAll();
	void Package(char **buf);

	std::string topic_;
	std::vector<int> partitions_;
};

class MemberAssignment {
public:
	MemberAssignment();
	MemberAssignment(const std::string &topic, const std::vector<int> &partitions);
	MemberAssignment(char **buf);
	
	int CountSize();
	void PrintAll();
	void Package(char **buf);
	void ParsePartitions(std::vector<int> &output_partitions);

	short version_;
	std::vector<PartitionAssignment> partition_assignment_;		// array
	std::string user_data_;		// bytes
};

#endif
