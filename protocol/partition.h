#ifndef _PARTITION_H_
#define _PARTITION_H_

class Partition {
public:
	Partition(int id, int leader);

	int id_;
	int leader_;
};

#endif
