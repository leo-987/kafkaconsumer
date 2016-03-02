#ifndef _REQUEST_RESPONSE_H_
#define _REQUEST_RESPONSE_H_

// Api key is short, so use struct rather than enum
struct ApiKey {
	static const short ProduceRequest          = 0;
	static const short FetchRequest            = 1;
	static const short OffsetRequest           = 2;
	static const short MetadataRequest         = 3;
	static const short OffsetCommitRequest     = 8;
	static const short OffsetFetchRequest      = 9;
	static const short GroupCoordinatorRequest = 10;
	static const short JoinGroupRequest        = 11;
	static const short HeartbeatRequest        = 12;
	static const short LeaveGroupRequest       = 13;
	static const short SyncGroupRequest        = 14;
	static const short DescribeGroupsRequest   = 15;
	static const short ListGroupsRequest       = 16;
};

#endif
