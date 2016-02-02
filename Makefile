
CPPFLAG = -Wall -std=c++11
run_main: main
	./main

main: main.cc kafka_client.cc network.cc request.cc response.cc util.cc blocking_queue.cc node.cc net_util.cc state_machine.cc
	g++ $(CPPFLAG) -o $@ $^ -lev -lpthread -g


clean:
	rm main -rf

