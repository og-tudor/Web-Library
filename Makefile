CC=g++
CFLAGS=-g -Wall -Wextra -Werror

client: client.cpp requests.cpp helpers.cpp buffer.cpp commands.cpp
	$(CC) $(CFLAGS) -o client client.cpp requests.cpp helpers.cpp buffer.cpp commands.cpp

run: client
	./client
debug: client
	valgrind --leak-check=full --show-leak-kinds=all ./client

clean:
	rm -f *.o client
