#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>

#define Lab6Handlers 128

#define Lab6ErrorThreading 2

#define Lab6NameSize 32

#ifndef byte
#define byte unsigned char
#endif

#ifndef null
#define null NULL
#endif

struct Message {
	char * Name;
	char Body[80];
};

struct Room {
	char Title[80];
	struct Message **Messages;
	int MessagesCount;
};

struct Room **Rooms;

struct Handle {
	pthread_t Thd;
	int Desc;
	byte Enabled;
};

struct Handle Handlers[Lab6Handlers] = {0};

void * Handler(void * Arg) {
	struct Room * RoomCur;
	int Desc = *((int *)Arg);
	char Name[Lab6NameSize] = {0};
	char Data[80] = {0};
	//byte Dat;
	byte State = 0;
	int i;
	//struct Room *R;

	while (1) {
		switch (State) {
		case 0:
			if (read(Desc, Name, Lab6NameSize)) {
				write(1, "New client name: \"", 18);
				write(1, Name, strlen(Name)-1);
				write(1, "\"\n",2);
				State++;
			}
			break;
		case 1:
			write(Desc, "\033[2J", sizeof("\033[2J"));
			write(Desc, "Awailable rooms:\n", sizeof("Awailable rooms:\n"));

			for(i=0; Rooms[i]; i++) {
				sprintf(Data, "[%d]%s", i, Rooms[i]->Title);
				write(Desc, Data, strlen(Data));
			}

			write(Desc, "Choose the room or type \">\" to create one.\n>:",
			     sizeof("Choose the room or type \">\" to create one.\n>:") - 1);

			while(1) {
				memset(Data, 0, 80);
				if(read(Desc, Data, 80)) {
					//write(1, Data, 80);
					if(Data[0] == '>' && Data[1] == '\n') {
						State = 2;
						break;
					} else {
						int RoomCount;

						for(RoomCount=0; Rooms[RoomCount]; RoomCount++);

						if(atoi(Data) < RoomCount){
							RoomCur = Rooms[atoi(Data)];
							State = 3;
							break;
						}
					}
				}
			}
			break;
		case 2:
                        write(Desc, "\033[2J", sizeof("\033[2J"));

			for(i=0; Rooms[i]; i++);

			Rooms = realloc(Rooms, sizeof(struct Room *) * (i+2));
			Rooms[i+1] = null;
			Rooms[i] = (struct Room *)malloc(sizeof(struct Room));

			write(Desc, "New room Title:\n>:", sizeof("New room Title:\n>:"));
                        while(1) {
				memset(Rooms[i]->Title, 0, 80);
                                if(read(Desc, Rooms[i]->Title, 80)) {
						break;
                                }
                        }
			write(Desc, "New room Body:\n>:", sizeof("New room Body:\n>:"));
			while(1) {
				Rooms[i]->Messages = malloc(sizeof(struct Message *));
				Rooms[i]->MessagesCount = 1;
				Rooms[i]->Messages[0] = calloc(1, sizeof(struct Message));
				Rooms[i]->Messages[0]->Name = Name;

                                if(read(Desc, Data, 80)) {
						*strchr(Data, '\n') = 0;
						strcpy(Rooms[i]->Messages[0]->Body, Data);
                                                break;
                                }
                        }
			write(Desc, "Done.\n", sizeof("Done.\n"));
			RoomCur = Rooms[i];
			State++;
                        break;
		case 3:
			fcntl(Desc, F_SETFL, fcntl(Desc, F_GETFL, 0) | O_NONBLOCK);
			while(1) {
				int j;
				//int CountPrev;
				int i;
				write(Desc, "\033[2J", sizeof("\033[2J"));
				write(Desc, "[", 1);
				write(Desc, RoomCur->Title, strlen(RoomCur->Title)-1);
				write(Desc, "]\n", 2);
				for(j = 0; j < (RoomCur->MessagesCount); j++){
					write(Desc, RoomCur->Messages[j]->Name, strlen(RoomCur->Messages[j]->Name)-1);
					write(Desc, ">:", 2);
					write(Desc, RoomCur->Messages[j]->Body, strlen(RoomCur->Messages[j]->Body));
					write(Desc, "\n", 1);
				}

				while(j == RoomCur->MessagesCount) {
					memset(Data, 0, 80);
        				if(read(Desc, Data, 80) > 1) {
						if(Data[0] == '<' && Data[1] == '\n') {
							State = 1;
							fcntl(Desc, F_SETFL, fcntl(Desc, F_GETFL, 0) &(~O_NONBLOCK));
							goto KEK;
						}
						char * Bug;
						RoomCur->Messages = realloc(RoomCur->Messages, sizeof(struct Message *) * (RoomCur->MessagesCount+1));
						RoomCur->Messages[RoomCur->MessagesCount] = malloc(sizeof(struct Message));
                		                RoomCur->Messages[RoomCur->MessagesCount]->Name = Name;
						Bug = strchr(Data, '\n');
						*Bug = 0;
						strcpy(RoomCur->Messages[RoomCur->MessagesCount]->Body, Data);
						RoomCur->MessagesCount++;
					}
				}
			}
			KEK:
			break;
		}
	}
}

int main() {
	int server_sockfd;
	int client_sockfd;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
	int server_len;
	int client_len;

	Rooms = malloc(sizeof(struct Room *));
        Rooms[0] = null;

	//unlink("Lab6");
	server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr("192.168.1.87");
	server_address.sin_port = 9734;

	server_len = sizeof(server_address);
	bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
	listen(server_sockfd, Lab6Handlers);
	while(1) {
		char ch;

		printf("Welcome\n");

		client_len = sizeof(client_address);
		while (1) {
			int NewDesc = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
			int i = 0;

			printf("New client connected.\n");


			while(Handlers[i].Enabled)
				i++;

			Handlers[i].Desc = NewDesc;

			if (pthread_create(&(Handlers[i].Thd), null, Handler, (void *)&(Handlers[i].Desc))) {
                        	printf("Error: Unable to create Thread.\n");
                        	return Lab6ErrorThreading;
                	}
		}
	}
}
