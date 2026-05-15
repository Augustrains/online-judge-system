talk: talk.cc
	g++ -o $@ $^ -std=c++11 -I/usr/include/jsoncpp $(shell mariadb_config --include) -ljsoncpp $(shell mariadb_config --libs) -lpthread
.PHONY:clean
clean:
	rm -f talk
