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
//structura care va fi primita de clientul udp
struct udp_message
{
	char topic[50];
	unsigned char type;
	char content[1500];
}udpm;

//folosit pentru arrayul de structuri topic
struct nodeClient
{
	int socket;
	char *id;
	struct nodeClient * next;
};

struct clientAddr
{
	int port;
	char *ip;
	char *id;
};
//initializeaza client nou
struct nodeClient* newClient(char* client_name, int socket)
{
	struct nodeClient* n = malloc(sizeof(struct nodeClient));
	n->id = malloc(sizeof(char)*11);
	strncpy(n->id , client_name, sizeof(client_name));
	n->next = NULL;
	n->socket = socket;
	return n;
}
//topicul este format din nume si lista inalntuiota de clienti
struct topic 
{
	char* name;
	struct nodeClient* clients;
	int SF;
};
//initializare topic
struct topic* initTopic(char* name, int SF)
{
	struct topic *t;
	t = malloc(sizeof(struct topic));
	t->name = malloc(sizeof(char)*50);
	strncpy(t->name , name, sizeof(name));
	t->SF = SF;
	t->clients = NULL;
	return t;
}
//adaugare client la topic
void addClient(struct topic *t, char* id, int socket)
{
	struct nodeClient *crawl;
	//crawl = t->clients;
	if(t->clients == NULL)
	{
		t->clients = newClient(id, socket);
		return;
	}
	crawl = t->clients;
	while(crawl->next!=NULL)
	{
		crawl=crawl->next;
	}
	crawl->next = newClient(id, socket);
	return;
}
//eliberare client (functionalitate ,din pacate, incompleta)
void freeClient(struct topic *t, char* id)
{
	int i = 0 ;
	struct nodeClient *n = t->clients;
	if(n == NULL)
	{
		return;
	}
	if(strcmp(n->id , id))
	{
		struct nodeClient *aux = n->next;
		free(n);
		t->clients = aux;
		return;
	}
	while(n->next != NULL)
	{
	if(strcmp(n->next->id , id))
		{
			struct nodeClient *aux = n->next->next;
			free(n->next);
			n->next = n->next->next;
			return;
		}
		n = n->next;
	}
	return;
}

int main(int argc, char const *argv[])
{
	//all purpuse buffer
	char buff[2000];
	int buff_size = 2000;
	char * udp_buff;
	udp_buff = malloc(sizeof(struct udp_message));
	struct sockaddr_in addr_tcp, addr_udp, addr;
	int sockfd_tcp, sockfd_udp, aux_sock, max_fd;
	//flags
	fd_set active_fds, msg;
	int sizeaddr = sizeof(struct sockaddr);
	int udp_size = sizeof(struct sockaddr_in);
	//mesaj udp
	struct udp_message * message;

	//array-ul de structuri de topic 
	struct topic **t;
	t = malloc(50 * sizeof(struct topic));
	int topic_size = 4;

	//stocarea adreselor clientilor
	struct clientAddr ** adr;
	adr = malloc (sizeof(struct clientAddr) * 50);
	int client_nr=0;
	//in cazul ca nu sunt numarul corespunbzator de inputuri
	if(argc != 2)
	{
		printf("Rularea se face in urmotorul fel:\n ./server <PORT_DORIT>\n");
		return -1;
	}	
	
	if(atoi((argv[1])) <= 1024)
	{
		printf("Numarul de port rezervat pentru alte aplicatii\n");
		return -1;
	}
	//initializare socket tcp
	sockfd_tcp = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd_tcp == -1)
	{
		printf("Nu s-a reusit sa se creeze un socket TCP");
		return -1;
	}
	//initializare socket udp
	sockfd_udp = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd_udp == -1)
	{
		printf("Nu s-a reusit sa se creeze un socket UDP");
		return -1;
	}
	//completare de adrese pt tcp / udp
	memset((char *) &addr_tcp, 0, sizeof(addr_tcp));
	addr_tcp.sin_family = AF_INET;
    addr_tcp.sin_port = htons(atoi(argv[1]));
    addr_tcp.sin_addr.s_addr = INADDR_ANY;

    memset((char *) &addr_udp, 0, sizeof(addr_udp));
    addr_udp.sin_family = AF_INET;
    addr_udp.sin_port = htons(atoi(argv[1]));
    addr_udp.sin_addr.s_addr = INADDR_ANY;
    //leaga socketele
    if(bind(sockfd_tcp, (struct sockaddr *) &addr_tcp, sizeof(struct sockaddr)) == -1)
    {
    	perror("socket error");
    	printf("Cannot bind TCP\n");
    	return -1;
    }
    if(bind(sockfd_udp, (struct sockaddr *) &addr_udp, sizeof(struct sockaddr)) == -1)
    {
    	perror("socket error");
    	printf("Cannot bind UDP\n");
    	return -1;
    }
    //setarea flagurilor pentru functia select (ce se ocupa de multiplexare)
    FD_ZERO(&active_fds);
    FD_ZERO(&msg);
    FD_SET(STDIN_FILENO, &active_fds);
    FD_SET(sockfd_tcp, &active_fds);
    FD_SET(sockfd_udp, &active_fds);
    FD_SET(0, &active_fds);
    max_fd = sockfd_udp;
  	if(STDIN_FILENO > max_fd)
	{
		max_fd = STDIN_FILENO;
	}
    if(listen(sockfd_tcp, 999999) == -1)
    {
    	perror("listen");
    }
	while(1)
	{
		//Multiplexare
		if(select(max_fd + 1, &active_fds, NULL, NULL, NULL) == -1)
		{
			return -1;
		}
		for(int i=0; i < max_fd + 1 ; i++)
		{	
			//functia de exit
			if(FD_ISSET(i, &active_fds) != 0)
			{
				if(i == STDIN_FILENO)
				{
					memset(buff, 0x0, buff_size);
					fgets(buff, buff_size - 1, stdin);
					if(strncmp(buff, "exit", 4) == 0)
					{
						return 0;
					}
				}
				//daca este primit un mesaj de la un UDP
				if(i == sockfd_udp)
				{
					//primeste mesajul
					int rcv = recvfrom(sockfd_udp, udp_buff, sizeof(struct udp_message), 0, (struct sockaddr *) &addr_udp, &sizeaddr);
					if( rcv > 0)
					{
						//"casteaza" mesajul
					 	memcpy (&message, &udp_buff, sizeof(struct udp_message));
					 	for(int j = 0; j < topic_size; j++)
						{
							//daca topicul exista, trimite mesaj la fiecare client
							if((strcmp(t[j]->name, message->topic) == 0))
							{
								//type-ul
								int type = (int)message->type;
								char typechr[10];
								if(type == 0)
								{
									char typechr[] = "INT";
								}
								if(type == 1)
								{
									char typechr[] = "SHORT_REAL";	
								}
								if(type == 2)
								{
									char typechr[] = "FLOAT";	
								}
								if(type == 3)
								{
									char typechr[] = "STRING";
								}
								struct nodeClient * pc = t[j]->clients; 
								while(pc->next!=NULL)
								{
									//formateaza mesaju;
									sprintf(buff, "%s:%d %s - %s - %s - %s",inet_ntoa(addr_udp.sin_addr),
									addr_udp.sin_port, pc->id, t[j]->name, typechr, message->content);
									//trimite
									send(pc->socket, buff, sizeof(buff) - 1, 0);
									//goleste bufferul
									memset(buff, 0x0, buff_size);
									pc = pc->next;
								}

							}
						}
						//goleste mesajul
						memset(message, 0x0, sizeof(udpm));
					}
					else
					{
						perror("udp mesage");
					}
					if(aux_sock > max_fd) {
                        max_fd = sockfd_tcp;
                    }
				}
				//primeste conexiune de la tcp
				if(i == sockfd_tcp)
				{
					//accepta conexiune
					aux_sock = accept(sockfd_tcp, (struct sockaddr *) &addr, &sizeaddr);
					if(aux_sock != -1)
					{
						//adauga noul socket la setul de socketi
						FD_SET(aux_sock, &active_fds);
						if( recv(aux_sock, buff, buff_size - 1, 0) != -1)
						{
							//adauga la lista de clienti
							adr[client_nr] = malloc(sizeof(struct clientAddr));
							adr[client_nr]->id = malloc(sizeof(buff));
							strncpy(adr[client_nr]->id, buff, buff_size);
							adr[client_nr]->ip = malloc(sizeof(inet_ntoa(addr.sin_addr)));
							strncpy(adr[client_nr]->ip, inet_ntoa(addr.sin_addr), sizeof(inet_ntoa(addr.sin_addr)) + 1);
							adr[client_nr]->port = ntohs(addr.sin_port);
							printf("New client %s connected from %s:%d.\n",adr[client_nr]->id, adr[client_nr]->ip,adr[client_nr]->port);
							client_nr++;
						}
						memset(buff, 0x0, buff_size);
					}
					else
					{
						perror("connection error");
					}
					if(aux_sock > max_fd) {
                        max_fd = aux_sock;
                    }
				}
				else
				{
					//primeste mesaj de la tcp
					int size_recv = recvfrom(i, buff, buff_size - 1, 0, (struct sockaddr *) &addr, &sizeaddr);
					if( size_recv != -1 ||size_recv != 0)
					{
						char aux_buff[256];
						char * subscription;
						char * topic;
						int SF; 
						strncpy(aux_buff, buff, buff_size);
						char* rest = aux_buff;
						//token-izeaza mesajul
						subscription = strtok_r(rest, " ", &rest);
						if(strcmp(subscription, "subscribe") == 0)
						{
							for(int i=0; i< client_nr; i++)
							{
								//daca adresa ip si portul corespun clientului
								if( (strcmp(adr[i]->ip, inet_ntoa(addr.sin_addr)) == 0) && (adr[i]->port == ntohs(addr.sin_port)) )
								{
									topic = strtok_r(rest, " ", &rest);
									SF = atoi(strtok_r(rest, " ", &rest));
									for(int j = 0; j < topic_size; j++)
									{
										//adauga la lista de topic-uri clientul
										if((strcmp(t[j]->name, topic) == 0))
										{
											addClient(t[j], adr[i]->id, i);
										}
									}
								} 
							}
						}
						memset(aux_buff, 0x0, buff_size);
						memset(buff, 0x0, buff_size);
					}
					else
					{
						perror("subscription error");
					}
				}
			}
		}
	}
	return 0;
}