pagerank: pagerank.cpp
	g++ -o pagerank pagerank.cpp -O3 -std=c++11 -pthread

clean:
	rm -f pagerank