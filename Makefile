.PHONY: build clean install uninstall

CXX=g++
CXXFLAGS=-Wall -Wextra

bulid: cpp/main.cpp client.go
	$(CXX) cpp/main.cpp $(CXXFLAGS) -o calculator	
	go build client.go 

clean:
	rm -f calculator client
