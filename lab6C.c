#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <resolv.h>
#include <netinet/in.h>
#include <pthread.h>

#define Lab6ErrorConnect 1

#ifndef null
#define null NULL
#endif

int sockfd;

void * KeyboardHandler (void * Arg) {
	char Data[80];
	while(1) {
		if(read(2, Data, 80)){
			write(sockfd, Data, strlen(Data));
			//write(1, Data, strlen(Data));
		}
	}

}

int main()
{
	//int sockfd;
	int len;
	struct sockaddr_in address;
	int result;
	char ch = 'A';
	char Dat;

	//int flags;

	pthread_t KeyboardHandlerThread;

	char Name[32] = {0};
	char Data[80] = {0};

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr("192.168.1.87");
	address.sin_port = 9734;

	len = sizeof(address);
	result = connect(sockfd, (struct sockaddr *)&address, len);

	if (result == -1) {
		printf("Error: unable to connect");
		return Lab6ErrorConnect;
	}

	printf("Welcome!\nYour name:\n>:");

	read(2, Name, 32);

	write(sockfd, Name, strlen(Name));

	//flags = fcntl(2, F_GETFL, 0);
	//fcntl(2, F_SETFL, flags | O_NONBLOCK);

	pthread_create(&(KeyboardHandlerThread), null, KeyboardHandler, null);

//	read(sockfd, &ch, 1);

//	printf("Rx:%c", ch);

	while(1) {
		if (read(sockfd, &Dat, 1))
			write(1, &Dat, 1);

 	}

	close(sockfd);

	return 0;
}
