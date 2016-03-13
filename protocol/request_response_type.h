#ifndef _REQUEST_RESPONSE_H_
#define _REQUEST_RESPONSE_H_

// Api key is short, so use struct rather than enum
struct ApiKey {
	static const short ProduceType          = 0;
	static const short FetchType            = 1;
	static const short OffsetType           = 2;
	static const short MetadataType         = 3;
	static const short OffsetCommitType     = 8;
	static const short OffsetFetchType      = 9;
	static const short GroupCoordinatorType = 10;
	static const short JoinGroupType        = 11;
	static const short HeartbeatType        = 12;
	static const short LeaveGroupType       = 13;
	static const short SyncGroupType        = 14;
	static const short DescribeGroupsType   = 15;
	static const short ListGroupsType       = 16;
};

#endif
