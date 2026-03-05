weather: weather.cc
	g++ -o $@ $^ -std=c++11  -L/usr/lib64/mysql  -ljsoncpp -lmysqlclient -lpthread
.PHONY:clean
clean:
	rm -f weather