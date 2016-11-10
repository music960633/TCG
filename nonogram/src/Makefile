all:
	g++ -O2 main.cpp -o solver
P:
	g++ -O2 -DPREPROCESS main.cpp -o solver_P
I:
	g++ -O2 -DIMPLICATION main.cpp -o solver_I
PI:
	g++ -O2 -DPREPROCESS -DIMPLICATION main.cpp -o solver_PI
clean:
	rm *.o solver
