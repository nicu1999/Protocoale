#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include<errno.h>
#include <netinet/tcp.h>

int main(int argc, char const *argv[])
{
	char word[256];
	char buff[256];
	int buff_size = 256;
	int sockfd_tcp;
	struct sockaddr_in sub_addr;
	fd_set active_fds;
	int sizeaddr = sizeof(struct sockaddr);
	struct sockaddr_in addr;
	//vberificare rulare corespunzatoare
	if(argc != 4)
	{
		printf("Rularea se face in urmotorul fel:\n ./subscriber <ID_Client> <IP_Server> <Port_Server>\n");
		return -1;
	}	
	//deschide socketul
	sockfd_tcp = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd_tcp == -1)
	{
		printf("Nu s-a reusit sa se creeze un socket");
		return -1;
	}
	//seteaza adresele
    sub_addr.sin_family = AF_INET;
    sub_addr.sin_port = htons(atoi(argv[3]));
    FD_ZERO(&active_fds);
    FD_SET(sockfd_tcp, &active_fds);
    FD_SET(0, &active_fds);
    if(inet_aton(argv[2], &sub_addr.sin_addr) == 0)
    {
    	printf("Convertire nereusita");
    	return -1;
    }//se conecteaza
    if (connect(sockfd_tcp, (struct sockaddr *) &sub_addr, sizeof(sub_addr)) == -1)
    {
    	perror("connection error");
		printf("Nu s-a reusit sa se creeze conexiunea\n");
		return -1;
    }//trimitere id catre server
    if(send(sockfd_tcp, argv[1], sizeof(argv[1]), 0) == -1)
    {
    	perror("send error");
    	printf("Trimitere id nereusita\n");
    }//dezactiveaza algoritmul lui Nagle
    setsockopt(sockfd_tcp, IPPROTO_TCP, TCP_NODELAY, (char *)1, sizeof(int));
	while(1)
	{
		if(select(sockfd_tcp, &active_fds, NULL, NULL, NULL) == -1)
		{
			return -1;
		}
		//primeste mesaj de la server
		if(FD_ISSET(sockfd_tcp, &active_fds) != 0)
		{
			int size_recv = recv(sockfd_tcp, buff, buff_size - 1, 0);
			if( size_recv != -1 ||size_recv != 0)
				{
					printf("%s\n", buff);
					memset(buff, 0x0, buff_size);
				}
		}
		fgets(word, sizeof(word), stdin);
		//iese din client
		if(strncmp(word, "exit",strlen("exit")) == 0)
		{
			close(sockfd_tcp);

			return 0;
		}
		//daca comenzile sunt subscribe respect unsubscribe
		//trimite la server
		else if(strncmp(word, "subscribe", strlen("subscribe")) == 0)
		{
			if (send(sockfd_tcp, word, sizeof(word) - 1, 0) != -1)
			{
				printf("subscribed\n");
				memset(word, sizeof(word), 0);
			}
		}else if(strncmp(word, "unsubscribe", strlen("unsubscribe")) == 0)
		{
			if (send(sockfd_tcp, word, sizeof(word) - 1, 0) != -1)
			{
				printf("unsubscribed\n");
				memset(word, sizeof(word), 0);
			}
		}
	}
	return 0;
}