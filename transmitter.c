#include "dcomm.h"

FILE* file;	
Byte sign;
Byte *bufSign;

int main(int argc, char const *argv[])
{
	struct sockaddr_in remaddr;
	int sockfd, i, slen=sizeof(remaddr);
	int recvlen;		/* # bytes in acknowledgement message */
	char *server;	/* change this to use a different server */
	char *filename;
	char ch;
	int port;
	int flagXOFF = 0, flagXON = 1;

	/* Initialize XON/XOFF */
	sign = XON;

	/* Assign server & port */
	server = argv[1];
	port = atoi(argv[2]);

	/* Create a Socket */
	if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
		perror("Socket Created");
		return 0;
	}
	else{
		printf("Membuat socket untuk koneksi %s : %d\n",server,port);
	}

	/* Bind Socket */	
	memset((char *) &remaddr, 0, sizeof(remaddr));
	remaddr.sin_family = AF_INET;
	remaddr.sin_port = htons(port);
	if (inet_aton(server, &remaddr.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}

	/* Create Child Process */
	int pid;
	pid = fork();

	if(pid == 0)	/* Child Process */
	{
		while(1){
			recvlen = recvfrom(sockfd, bufSign, MAXLEN, 0, (struct sockaddr *)&remaddr, &slen);
			if(recvlen >= 0){
				printf("receive message: %s\n", bufSign);

				sign = *bufSign;
				if(sign == XOFF && flagXOFF == 1){
					printf("XOFF diterima\n");
					flagXON = 1;
					flagXOFF = 0;
				}
				else if(sign == XON && flagXON == 1){
					printf("XON diterima\n");
					flagXON = 0;
					flagXOFF = 1;
				}
			}
		}
	}
	else{		/* Parent Process */
		/* Open External File */
		filename = argv[3];
		file = fopen(filename, "r");
		if(file == NULL){
			perror("File error\n");
			return 0;
		}

		i = 0;
		int scanfile = fscanf(file, "%c", &ch);
		while(scanfile > 0){ // while not end of file
			if(sign == XON){
				i++;
				printf("Mengirim byte ke-%d: '%c'\n", i, ch);
				//printf("Sign: %s\n", sign);

				if(sendto(sockfd, &ch, 1, 0, (struct sockaddr *)&remaddr, slen) == -1){
					perror("Failed to Send");
					return 0;
				}
				scanfile = fscanf(file, "%c", &ch);
			}
			else{
				printf("Menunggu XON...\n");
			}
			usleep(DELAY*1000);
		}
		fclose(file);
		close(sockfd);
	}

	return 0;
}