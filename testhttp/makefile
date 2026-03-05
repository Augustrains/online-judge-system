test:test.cc
	g++ -o $@ $^ -std=c++11 -L/usr/lib64/mysql  -lpthread -lctemplate -ljsoncpp  -lmysqlclient
.PHONY:clean
clean:
	rm -f test