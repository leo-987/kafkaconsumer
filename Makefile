CC = g++
TARGET = main
CXXFLAGS = -std=c++11 -c -Wall -g
LDFLAGS = -lev -lpthread
SOURCES = main.cc kafka_client.cc network.cc request.cc response.cc\
		  util.cc blocking_queue.cc node.cc net_util.cc partition.cc\
		  member_assignment.cc message_set.cc #state_machine.cc

OBJECTS = $(SOURCES:.cc=.o)

all: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJECTS) 
	$(CC) -o $@ $(LDFLAGS) $(OBJECTS)

%.o: %.cc
	$(CC) -o $@ $(CXXFLAGS) $<

clean:
	rm -rf main *.o
