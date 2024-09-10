#include "readerwriterqueue.h"
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

#define PORT 6123
#define BUFFERSIZE 950000

void readEquipment(char * buffer, moodycamel::ReaderWriterQueue<int> * queue);
void readPackets(char * buffer, moodycamel::ReaderWriterQueue<int> * queue);

int main() {
  moodycamel::ReaderWriterQueue<int> q(100); // Reserve space for at least 100 elements up front
  char buffer[BUFFERSIZE];
  std::thread t1(readEquipment, buffer, &q);
  std::thread t2(readPackets, buffer, &q);
  t1.join();
  return 0;
}

void readPackets(char * buffer, moodycamel::ReaderWriterQueue<int> * queue) {
    while (true){
        int position;
        if (queue->try_dequeue(position)) {
            printf("2-position: %d\n", position);
            for (int i = position; i < position + 9500; i++) {
                printf("%02x ", buffer[i]);
            }
            std::cout << std::endl;
        }
    }
}

void readEquipment(char * buffer, moodycamel::ReaderWriterQueue<int> * queue) {
  int sockfd;
  int position = 0;
  const char *hello = "Hello from server";
  struct sockaddr_in servaddr, cliaddr;

  // Creating socket file descriptor
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  memset(&cliaddr, 0, sizeof(cliaddr));

  // Filling server information
  servaddr.sin_family = AF_INET; // IPv4
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(PORT);

  // Bind the socket with the server address
  if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  socklen_t len;
  int n;

  len = sizeof(cliaddr); // len is value/result

  // Store equipment IP
  struct sockaddr_in equipmentIP;
  equipmentIP.sin_addr.s_addr = inet_addr("127.0.0.1");

  // Read the message from the client into the buffer
  while (true) {
    n = recvfrom(sockfd, (char *)(buffer + position), BUFFERSIZE, MSG_WAITALL,
                 (struct sockaddr *)&cliaddr, &len);

    if (equipmentIP.sin_addr.s_addr == cliaddr.sin_addr.s_addr) { // check if the IP is the same
      queue->enqueue(position);
       printf("1-position: %d\n", position);
      for (int i = position; i < position + 9500; i++) {
//        printf("%02x ", buffer[i]);
        // printf("position %d: %02x\n", position, buffer[i]);
      }
     // std::cout << std::endl;
      position += 9500;
      if (position >= BUFFERSIZE) {
        position = 0;
      }
    }
  }
}