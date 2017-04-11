all: q r h b g

q: quicksort.cpp
	g++ -std=c++11 -pthread -o qs quicksort.cpp

r: radix.cpp
	g++ -std=c++11 -pthread -o rs radix.cpp 

h: hash_p.cpp
	g++ -std=c++11 -pthread -o hs hash_p.cpp

b: bitonic.cpp
	g++ -std=c++11 -pthread -o bs bitonic.cpp

g: gaussian.cpp
	g++ -std=c++11 gaussian.cpp -o  gau -lpthread
