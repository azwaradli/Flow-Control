/*
* File : receiver.cpp
*/
#include "dcomm.h"
/* Delay to adjust speed of consuming buffer, in milliseconds */
#define DELAY 500
/* Define receive buffer size */
#define RXQSIZE 8
/* Define minimum upperlimit */
#define MIN_UPPERLIMIT 4
/* Define maximum lowerlimit */
#define MAX_LOWERLIMIT 1

Byte rxbuf[RXQSIZE];
QTYPE rcvq = { 1, 0, 0, RXQSIZE, rxbuf };
QTYPE *rxq = &rcvq;
Byte sendControlChar = XON;
/* Socket */
int sockfd; // listen on sock_fd
struct sockaddr_in myaddr;	/* our address */
struct sockaddr_in remaddr;	/* remote address */
socklen_t addrlen = sizeof(remaddr);		/* length of addresses */

/* Functions declaration */
void* mainThread(void* threadArgs);
/*

*/

static Byte *rcvchar(int sockfd, QTYPE *queue);
/*
Read a character from socket and put it to the receive buffer.
If the number of characters in the receive buffer is above certain
level, then send XOFF and set a flag (why?).
Return a pointer to the buffer where data is put.
*/

static Byte *q_get(QTYPE *, Byte *);
/*
Retrieve data from buffer, save it to "current" and "data"
If the number of characters in the receive buffer is below certain
level, then send XON.
Increment front index and check for wraparound.
q_get returns a pointer to the buffer where data is read or NULL if
buffer is empty.
*/

int isValidChar(char inChar);
/*
Check whether character is  a valid character? not ( >32, CR, LF atau end-of-file)
*/

void insertToQueue(QTYPE* Q, Byte input);
/*

*/

void getFromQueue(QTYPE* Q, Byte* output);
/*

*/

// Global Variable
static int countRcvdBytes = 0;   // number of Received Bytes
static int countCnsmdBytes = 0;   // number of Consumed Bytes

Byte buffer[2];
Byte currentByte;
int main(int argc, char *argv[]){

  //check whether argument is valid or not
	if (argc != 2) {
		printf("Parameter(s)" "<Server Port/Service>\n");
		return 0;
	}

	// save the port
	int servicePort = atoi(argv[1]);

  // create a UDP socket
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("cannot create socket\n");
    return 0;
  }

  // bind the socket to any valid IP address and a specific port
  memset((char *)&myaddr, 0, sizeof(myaddr));
  myaddr.sin_family = AF_INET;
  myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  myaddr.sin_port = htons(servicePort);

  if (bind(sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
    perror("bind failed");
    return 0;
  }

  int pid;
  pid = fork();
  if(pid == 0)  /* Child Process */
  {
    while(1){
      printf("%d\n",rxq->count);
      printf("%c\n",rxq->data[countRcvdBytes]);
      //printf("test\n");
      //printf("%c\n", *q_get(rxq,&currentByte));
      /*if(q_get(rxq,&currentByte)){
        printf("child\n");
        /*
        if(rxq->front > 0){
          current = rxq->data[rxq->front-1];
        }
        else{
          current = rxq->data[7];
        }
        */
        //printf("masuk\n");

        /*if(isValidChar(currentByte)){
          printf("Mengkonsumsi byte ke-%d: ‘%c’\n", countCnsmdBytes++, currentByte);
        }
        else if (currentByte == Endfile){
          exit(0);
        }*/
      //}
      usleep(DELAY*1000);
    }
  }
  else{
    printf("Binding pada 127.0.0.1:%d ...",servicePort);
    while (1) {
      //printf("parrent\n");
      currentByte = *(rcvchar(sockfd, rxq));
      // Quit on end of file
      if (currentByte == Endfile) {
        exit(0);
      }
      usleep(DELAY*1000);
    }
  }
}

Byte *rcvchar(int sockfd, QTYPE *queue){
  if(sendControlChar == XON){
    int lengthRcvdBytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&remaddr, &addrlen);

    if(lengthRcvdBytes >= 0){
      printf("Menerima byte ke-%d.\n",countRcvdBytes);
      //insertToQueue(&*queue,buffer[0]);
      (*queue).data[(*queue).rear] = buffer[0];
      (*queue).count++;
      (*queue).rear = (((*queue).rear) + 1) % (*queue).maxsize;
      
      countRcvdBytes++;
    }
    else{ //recvfrom() failed
      printf("recvfrom() does not get data");
    }

    if((*queue).count > MIN_UPPERLIMIT){
      printf("Buffer > minimum upperlimit. Mengirim XOFF.");
      sendControlChar = XOFF;
      char bufferSend[2];
      bufferSend[0] = sendControlChar;
      ssize_t lengthSentBytes;
      if((lengthSentBytes = sendto(sockfd, bufferSend, sizeof(bufferSend), 4,
        (struct sockaddr *)&remaddr, addrlen)) < 0){
        printf("sendto() failed)\n");
      }
    }
  }
  else{ // Send XOFF
    buffer[0] = 0;
  }
  return &buffer[0];
}


static Byte *q_get(QTYPE *queue, Byte *data){
  // Nothing in the queue
  //if (!(*queue).count) return (NULL);
  //else{
    printf("%d\n",queue->count);
    do{
      printf("test\n");  
      //getFromQueue(&*queue, &*data);
    }while((!isValidChar(*data) || *data >= 32));
    /*
    if(queue->count < MAX_LOWERLIMIT && sendControlChar == XOFF){
      sendControlChar = XON;
      char bufferSend[2];
      bufferSend[0] = sendControlChar;
      printf("Buffer < maksimum lowerlimit. Send XON\n");
      ssize_t lengthSentBytes;
      if( (lengthSentBytes = sendto(sockfd, bufferSend, sizeof(bufferSend), 4,
          (struct sockaddr *) &remaddr, addrlen)) < 0){
          printf("sendto() failed\n");
      }
    }
    */
  //}
  return data;
}

int isValidChar(char inChar){
  return (inChar != CR && inChar != Endfile && inChar != LF);
}

void insertToQueue(QTYPE *Q, Byte input){
  printf("hai\n");
  Q->data[Q->rear] = input;
  (Q->count)++;
  Q->rear = ((Q->rear) + 1) % Q->maxsize;
}

void getFromQueue(QTYPE* Q, Byte* output){
  (*output) = Q->data[Q->front];
  (Q->count)--;
  Q->front = ((Q->front) + 1) % Q->maxsize;
}
