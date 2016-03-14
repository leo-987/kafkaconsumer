CC = g++
TARGET = main
CXXFLAGS = -std=c++11 -c -Wall -g -I./ -I./protocol -I./utils
LDFLAGS = 
SOURCES = main.cc kafka_client.cc network.cc\
		  utils/util.cc node.cc utils/net_util.cc\
		  protocol/partition.cc\
		  protocol/request.cc protocol/response.cc\
		  protocol/member_assignment.cc protocol/message_set.cc\
		  protocol/offset_request.cc protocol/offset_response.cc\
		  protocol/offset_fetch_request.cc protocol/offset_fetch_response.cc\
		  protocol/fetch_request.cc protocol/fetch_response.cc\
		  protocol/metadata_request.cc protocol/metadata_response.cc\
		  protocol/heartbeat_request.cc protocol/heartbeat_response.cc\
		  protocol/sync_group_request.cc protocol/sync_group_response.cc\
		  protocol/join_group_request.cc protocol/join_group_response.cc\
		  protocol/group_coordinator_request.cc protocol/group_coordinator_response.cc\
		  broker.cc

OBJECTS = $(SOURCES:.cc=.o)

all: depend $(TARGET)
	./$(TARGET)

depend: .depend
.depend: $(SOURCES)
	rm -f ./.depend
	$(CC) $(CXXFLAGS) -MM $^ > ./.depend;
-include .depend

$(TARGET): $(OBJECTS) 
	$(CC) -o $@ $(LDFLAGS) $(OBJECTS)


%.o: %.cc
	$(CC) -o $@ $(CXXFLAGS) $<

clean:
	rm -rf main *.o protocol/*.o utils/*.o .depend

