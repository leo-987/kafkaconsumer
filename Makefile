
all: main

main: main.cc request.cc response.cc
	g++ -std=c++11 -o $@ $^ -lev -g

clean:
	rm main -rf
