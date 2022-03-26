run:
	g++ -std=c++17 main.cpp -llua5.3 -I /home/anhong/b/3rdparty/sol2/single/include -I /usr/include/lua5.3 -L /usr/lib/x86_64-linux-gnu -g -fsanitize=address -g

clean:
	rm a.out -rf
