
#include "partition.h"

Partition::Partition()
{
	id_ = -1;
	leader_ = -1;
}

Partition::Partition(int id, int leader)
{
	id_ = id;
	leader_ = leader;
}

int Partition::GetPartitionId()
{
	return id_;
}

int Partition::GetLeaderId()
{
	return leader_;
}
