search: my_socket.h board.h OTP.h search.cpp
	g++ -std=c++11 -DNDEBUG -O2 -Wall search.cpp -o R05921058
judge: my_socket.h board.h judge.cpp
	g++ -std=c++11 -O2 -Wall judge.cpp -o judge
http: my_socket.h board.h OTP.h simple_http_UI.cpp
	g++ -std=c++11 -DNDEBUG -O2 -Wall simple_http_UI.cpp -o simple_http_UI
test: R05921058 judge
	rm -rf log_ju.txt log_p1.txt log_p2.txt
	./judge 7122 > log_ju.txt &
	./R05921058 127.0.0.1 7122 > log_p1.txt &
	./R05921058 127.0.0.1 7122 > log_p2.txt &
