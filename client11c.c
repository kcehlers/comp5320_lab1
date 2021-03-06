#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <netdb.h>

#define SERV_PORT 10010
#define MAXLINE 1024
#define MAXDIGITS 5
#define NUM_AMOUNT 10000

void numToByteArray(long num, int size, char *bArray);
void buildPacket(int16_t msgLength, int32_t seqNum, long unsigned int timestamp, char *numString, char *sendPacket);
void decodePacket(char *receivePacket);

int main(int argc, char **argv) {
	int16_t msgLength;
	int32_t seqNum = 1;
	long unsigned int timestamp;
	char sendPacket[20];
	char receivePacket[20];
	char numString[5];
	int currentNumber = 1;
	//int avgRTT;
	//int maxRTT;
	//int minRTT;
	//long unsigned int returnTS;
	//int32_t returnSN;
	long timeout;
	long currTime;

		
	if (argc != 2) {
		perror("Usage: ./client11c <server hostname>");
		exit(1);
	}
	
	//int i;
	int socketfd;
	int n;
	socklen_t servlen;
	struct sockaddr_in servaddr;
	struct hostent *he;
	struct timeval start;
	
	gettimeofday(&start, NULL);
	timeout = (start.tv_sec * 1000 + start.tv_usec / 1000) + 5000;
	
	
	int pid;
	
	
	if ((he = gethostbyname(argv[1])) == NULL) {
		perror("gethostbyname");
		exit(1);
	}

	if ((socketfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("Problem in creating the socket");
		exit(2);
	}
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr = *((struct in_addr *)he->h_addr);
	servaddr.sin_port = htons(SERV_PORT);

	pid = fork();
	
	// Child - Receive data
	if (pid == 0) {
		while (1) {
			gettimeofday(&start, NULL);
			currTime = start.tv_sec * 1000 + start.tv_usec / 1000;
			// timeout:
			if (currTime > timeout) {
				// break out of loop 
				printf("\nReceiving timeout.\n");
				break;
			}

			servlen = sizeof(servaddr);
			if ((n = recvfrom(socketfd, (char *) receivePacket, 20, MSG_WAITALL, (struct sockaddr *) &servaddr, &servlen)) < 0) {
				perror("Error receiving packet");
				exit(4);
			}
			
			decodePacket(receivePacket);
			bzero(receivePacket, 20);
				

		}








		/*for (i = 0; i < NUM_AMOUNT; i++) {
			servlen = sizeof(servaddr);	
			if ((n = recvfrom(socketfd, (char *) numbersReceived[i], MAXDIGITS, MSG_WAITALL,(struct sockaddr *) &servaddr, &servlen)) < 0) {
				perror("Error in recvfrom");
				exit(4);
			}
			
			printf("Server: %s, index: %d\n", numbersReceived[i], i);
			// This is NOT a full proof method for ending the loop
			if (strcmp("10000\0", numbersReceived[i]) == 0) {
				break;
			}		
		}*/	
	}
	// Parent - Send data
	else {

		while (currentNumber < 10001) {
			printf("\nSending %d ...\n", currentNumber);
			sprintf(numString, "%d", currentNumber);
			
			// msg length = 14 + strlen(message)
			msgLength = 14 + strlen(numString);
			gettimeofday(&start, NULL);
			timestamp = start.tv_sec * 1000 + start.tv_usec / 1000;

			buildPacket(msgLength, seqNum, timestamp, numString, sendPacket);

			if (sendto(socketfd, (char *) sendPacket, sizeof(sendPacket), MSG_CONFIRM, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
				perror("Problem in sending the packet.");
				exit(3);
			}
			bzero(sendPacket, 20);
			seqNum++;
			currentNumber++;
			
		}
		
		/*for (i = 0; i < NUM_AMOUNT; i++) {
			printf("Sending %s\n", numbers[i]);

			if (sendto(socketfd, (char *) numbers[i], strlen(numbers[i]), MSG_CONFIRM, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
				perror("Error in sendto");
				exit(3);
			}
		}*/
	}

	close(socketfd);

	return 0;
}

void buildPacket(int16_t msgLength, int32_t seqNum, long unsigned int timestamp, char *numString, char *sendPacket) {
	char msgLengthArr[2];
	char seqNumArr[4];
	char timeStampArr[8];

	numToByteArray(msgLength, sizeof(msgLength), msgLengthArr);
	numToByteArray(seqNum, sizeof(seqNum), seqNumArr);
	numToByteArray(timestamp, sizeof(timestamp), timeStampArr);

	memcpy(sendPacket, msgLengthArr, sizeof(msgLengthArr));
	memcpy(sendPacket + sizeof(msgLengthArr), seqNumArr, sizeof(seqNumArr));
	memcpy(sendPacket + sizeof(seqNumArr) + sizeof(msgLengthArr), timeStampArr, sizeof(timeStampArr));
	memcpy(sendPacket + sizeof(timeStampArr) + sizeof(seqNumArr) + sizeof(msgLengthArr), numString, strlen(numString));
	sendPacket[19] = '\0';
}

void numToByteArray(long num, int size, char *bArray) {
	int i;
	for (i = 0; i < size; i++) {
		bArray[i] = (num >> (i * 8)) & 0xFF;
	}
}


void decodePacket(char *receivePacket) {
	int16_t msgLength = 0;
	int32_t seqNum = 0;
	//uint64_t timestamp = 0;
	int i;

	// Handle msgLength subarray:
	for (i = 1; i >= 0; i--) {
		msgLength += receivePacket[i];
		if (i > 0) {
			msgLength = msgLength << 8;	
		}
	}

	// Handle seqNum subarray:
	for (i = 5; i >= 2; i--) {
		seqNum += receivePacket[i];
		if (i > 2) {
			seqNum = seqNum << 8;
		}
	}

	// Handle timestamp subarray:
	/*for (i = 13; i >= 6; i++) {
		timestamp |= (long) receivePacket[i];
		if (i > 6) {
			timestamp = timestamp << 8;
		}
	}*/
	receivePacket[19] = '\0';
	printf("\nFrom the server:\n");
	printf("Message: %s\n", receivePacket + 14);
	printf("Packet Length: %d\n", msgLength);
	//printf("Timestamp: %ld\n", timestamp);
	printf("Sequence Number: %d\n", seqNum);
}
