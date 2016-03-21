#ifndef _PARTITION_H_
#define _PARTITION_H_

class Partition {
public:
	Partition();
	Partition(int id, int leader);

	int GetLeaderId();
	int GetPartitionId();

private:
	int id_;

	// If no leader exists because we are in the middle of a leader election this id will be -1
	int leader_;
};

#endif
