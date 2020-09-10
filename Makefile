all:
	g++ -o main src/*.cpp -I include -std=c++17 -O2 -Wall -Wextra -Wshadow -Wno-unused-result -Wno-sign-compare
debug:
	g++ -o main src/*.cpp -I include -std=c++17 -O2 -Wall -Wextra -Wshadow -Wno-unused-result -Wno-sign-compare -fsanitize=undefined -fno-sanitize-recover
head:
	g++ -o main src/*.cpp -I include -std=c++17 -O2 -Wall -Wextra -Wshadow -Wno-unused-result -Wno-sign-compare -fsanitize=undefined -fno-sanitize-recover 2>&1 | head -n 50
run:
	./main