LDFLAGS=-lwsock32
# LDFLAGS=-static -s 
.PHONY:	clean
search.exe:	main.cc anqi.cc ClientSocket.cpp Protocol.cpp
	$(CXX) -O2 -o $@ $^ $(LDFLAGS)
clean:
	rm search.exe
