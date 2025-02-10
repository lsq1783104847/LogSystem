example:example.cc
	g++ -o $@ $^ -std=c++17 -lpthread -g
.PHONY:clean
clean:
	rm example
