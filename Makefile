all:
	g++ -o bin/main src/*.cpp -I include -std=c++17 -O2 -Wall -Wextra -Wshadow -Wno-unused-result -Wno-sign-compare -Wno-format-zero-length -static
debug:
	g++ -o bin/main src/*.cpp -I include -std=c++17 -O2 -Wall -Wextra -Wshadow -Wno-unused-result -Wno-sign-compare -Wno-format-zero-length -fsanitize=undefined -fno-sanitize-recover -DDEBUG
run:
	./bin/main
