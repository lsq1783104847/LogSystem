all:example performance_test

example:example.cc
	g++ -o $@ $^ -std=c++17 -lpthread
performance_test:performance_test.cc
	g++ -o $@ $^ -std=c++17 -lpthread
.PHONY:clean
clean:
	rm example performance_test;rm -r data
