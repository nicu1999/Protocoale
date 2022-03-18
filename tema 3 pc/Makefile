build: client

client: client.cpp json.hpp Current.cpp helpers.h helpers.c
	g++ -Wall -o client client.cpp json.hpp Current.cpp helpers.h helpers.c buffer.h buffer.c

clean:
	rm -rf client
