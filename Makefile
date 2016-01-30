
#all: main

run_main: main
	./main

main: main.cc kafka_client.cc network.cc request.cc response.cc util.cc blocking_queue.cc
	g++ -std=c++11 -o $@ $^ -lev -lpthread -g


clean:
	rm main -rf
