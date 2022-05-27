run:
	g++ -std=c++17 main.cpp -llua -I /home/anhong/b/3rdparty/sol2/single/include -g -fsanitize=address -g

clean:
	rm a.out -rf
