test:test.cc
	g++ -o $@ $^ -std=c++17 -lpthread -g
.PHONY:clean
clean:
	rm test
