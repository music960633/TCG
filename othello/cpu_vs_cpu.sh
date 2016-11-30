g++ judge.cpp -std=c++11 -O2 -Wall -o judge
echo compile_judge_complete
g++ search.cpp -std=c++11 -O2 -Wall -o search
echo compile_search_complete
rm -rf log_ju.txt log_p1.txt log_p2.txt
./judge 7122 > log_ju.txt &
echo start_judge
./search 127.0.0.1 7122 > log_p1.txt &
echo start_search1
./search 127.0.0.1 7122 > log_p2.txt &
echo start_search2

