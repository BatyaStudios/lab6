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

#define Lab6Handlers 16

#define Lab6ErrorThreading 2

#define Lab6NameSize 32

#ifndef byte
#define byte unsigned char
#endif

#ifndef null
#define null NULL
#endif

struct Message {
	char *Name;
	char Body[80];
};

struct Room {
	char Title[80];
	struct Message **Messages;
	int MessagesCount;
};

struct Room **Rooms;

pthread_t Thd;
int Arg;

void SetNonBlockMode(int Desc)
{
	fcntl(Desc, F_SETFL, fcntl(Desc, F_GETFL, 0) | O_NONBLOCK);
}

void ClrNonBlockMode(int Desc)
{
	fcntl(Desc, F_SETFL, fcntl(Desc, F_GETFL, 0) & (~O_NONBLOCK));
}

byte Post(struct Room *RoomCur, byte *State, char *Data, char *Name, int Desc)
{
	if (read(Desc, Data, 80) > 1) {
		if (Data[0] == '<' && Data[1] == '\n') {
			*State = 1;
			ClrNonBlockMode(Desc);
			return 1;
			//goto KEK;
		}
		RoomCur->Messages = realloc(
		RoomCur->Messages,
		sizeof(struct Message *) * (RoomCur->MessagesCount+1));
		RoomCur->Messages[RoomCur->MessagesCount] =
		malloc(sizeof(struct Message));
		RoomCur->Messages[RoomCur->MessagesCount]->Name = Name;
		*strchr(Data, '\n') = 0;
		strcpy(RoomCur->Messages[RoomCur->MessagesCount]->Body, Data);
		RoomCur->MessagesCount++;
	}
	return 0;
}

void *Handler(void *Arg)
{
	struct Room *RoomCur;
	int Desc = *((int *)Arg);
	char Name[Lab6NameSize] = {0};
	char Data[80] = {0};
	byte State = 0;
	int i;

	while (1) {
		switch (State) {
		case 0:
			if (read(Desc, Name, Lab6NameSize)) {
				write(1, "New client name: \"", 18);
				write(1, Name, strlen(Name) - 1);
				write(1, "\"\n", 2);
				State++;
			}
			break;
		case 1:
			write(Desc, "\033[2J", sizeof("\033[2J"));
			write(Desc, "Awailable rooms:\n",
			     sizeof("Awailable rooms:\n") - 1);

			for (i = 0; Rooms[i]; i++) {
				sprintf(Data, "[%d]%s\n", i, Rooms[i]->Title);
				write(Desc, Data, strlen(Data));
			}

			write(Desc, "Choose the room",
			     sizeof("Choose the room") - 1);

			write(Desc, " or type \">\" to create one.\n>:",
			     sizeof(" or type \">\" to create one.\n>:") - 1);

			while (1) {
				if (read(Desc, Data, 80)) {
					int i;

					if (Data[0] == '>' &&
					    Data[1] == '\n') {
						State = 2;
						break;
					}
					for (i = 0; Rooms[i]; i++)
						;

					if (atoi(Data) < i) {
						RoomCur = Rooms[atoi(Data)];
						State = 3;
						break;
					}
				}
			}
			break;
		case 2:
			write(Desc, "\033[2J", sizeof("\033[2J"));

			for (i = 0; Rooms[i]; i++)
				;

			void *Check =
				 realloc(Rooms, sizeof(struct Room *) * (i+2));

			if (Check)
				Rooms = Check;
			else {
				write(Desc, "Unable to create new room.\n",
				     sizeof("Unable to create new room.\n")-1);
				State = 1;
				break;
			}

			Rooms[i+1] = null;
			Rooms[i] = (struct Room *)malloc(sizeof(struct Room));

			write(Desc, "New room Title:\n>:",
			     sizeof("New room Title:\n>:"));
			while (1) {
				if (read(Desc, Data, 80)) {
					*strchr(Data, '\n') = 0;
					strcpy(Rooms[i]->Title, Data);
					break;
				}
			}
			write(Desc, "New room Body:\n>:",
			     sizeof("New room Body:\n>:"));
			while (1) {
				Rooms[i]->Messages =
					      malloc(sizeof(struct Message *));
				Rooms[i]->MessagesCount = 1;
				Rooms[i]->Messages[0] =
					      calloc(1, sizeof(struct Message));
				Rooms[i]->Messages[0]->Name = Name;

				if (read(Desc, Data, 80)) {
					*strchr(Data, '\n') = 0;
					strcpy(Rooms[i]->Messages[0]->Body,
									  Data);
					break;
				}
			}
			write(Desc, "Done.\n", sizeof("Done.\n"));
			RoomCur = Rooms[i];
			State++;
			break;
		case 3:
			SetNonBlockMode(Desc);
			byte Quit = 0;

			while (!Quit) {
				int j;

				write(Desc, "\033[2J", sizeof("\033[2J"));
				write(Desc, "[", 1);
				write(Desc, RoomCur->Title,
						     strlen(RoomCur->Title)-1);
				write(Desc, "]\n", 2);
				for (j = 0; j < (RoomCur->MessagesCount);
									 j++) {
					write(Desc, RoomCur->Messages[j]->Name,
					 strlen(RoomCur->Messages[j]->Name)-1);
					write(Desc, ">:", 2);
					write(Desc, RoomCur->Messages[j]->Body,
					   strlen(RoomCur->Messages[j]->Body));
					write(Desc, "\n", 1);
				}

				while (j == RoomCur->MessagesCount) {
					Quit = Post(RoomCur,
						&State, Data, Name, Desc);
					j = (Quit ? -1:j);
				}
			}
			break;
		}
	}
}

int main(void)
{
	int server_sockfd;
	//int client_sockfd;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
	int server_len;
	int client_len;
	char Ip[16] = {0};

	Rooms = malloc(sizeof(struct Room *));
	Rooms[0] = null;

	//unlink("Lab6");
	server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

	server_address.sin_family = AF_INET;
	write(1, "Enter IP:(192.168.1.87)\n>:",
	  sizeof("Enter IP:(192.168.1.87)\n>:")-1);
	read(2, Ip, 15);
	//server_address.sin_addr.s_addr = inet_addr("192.168.43.176");
	server_address.sin_addr.s_addr = inet_addr(Ip);
	server_address.sin_port = 9734;

	server_len = sizeof(server_address);
	bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
	listen(server_sockfd, Lab6Handlers);
	while (1) {
		write(1, "Connected.\n", sizeof("Connected.\n")-1);

		client_len = sizeof(client_address);
		while (1) {
			int NewDesc = accept(server_sockfd,
			      (struct sockaddr *)&client_address, &client_len);

			printf("New client connected.\n");

			Arg = NewDesc;

			if (pthread_create(&Thd, null, Handler,
					(void *)&Arg)) {
				write(1, "Error: Unable to create Thread.\n",
				sizeof("Error: Unable to create Thread.\n")-1);
				return Lab6ErrorThreading;
			}
		}
	}
}
