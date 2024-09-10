#include "event.h"
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

void readEquipment(char *buffer, moodycamel::ReaderWriterQueue<std::pair<int, int>> *queue);
void readPackets(char *buffer, moodycamel::ReaderWriterQueue<std::pair<int, int>> *queue);

int main() {
  moodycamel::ReaderWriterQueue<std::pair<int, int>> q(100);
  char buffer[BUFFERSIZE];
  std::thread t1(readEquipment, buffer, &q);
  std::thread t2(readPackets, buffer, &q);
  t1.join();
  return 0;
}

std::vector<char> addEquipmentHeader(std::vector<char> *fragments, int length) {
  equipmentHeaderStruct equipmentHeader{
      .equipmentSize = equipmentSizeType(length + 7 * 4), // add header size
      .equipmentType = equipmentTypeType(23),             // add equipment type
      .equipmentId = equipmentIdType(2),                  // add equipment id
      .equipmentBasicElementSize = 4,
  };

  // print length
  printf("add header: length: %d\n", length);

  std::vector<char> equipmentData(length + 7 * 4);
  // copy(equipmentData[28:], subevent)
  //  Copy data flipping endianness
  for (int i = 0; i < length / 4; i++) {
    equipmentData[28 + i * 4] = (*fragments)[i * 4 + 3];
    equipmentData[28 + i * 4 + 1] = (*fragments)[i * 4 + 2];
    equipmentData[28 + i * 4 + 2] = (*fragments)[i * 4 + 1];
    equipmentData[28 + i * 4 + 3] = (*fragments)[i * 4];
  }
  std::memcpy(equipmentData.data(), &equipmentHeader.equipmentSize, sizeof(int));
  std::memcpy(equipmentData.data() + 4, &equipmentHeader.equipmentType, sizeof(int));
  std::memcpy(equipmentData.data() + 8, &equipmentHeader.equipmentId, sizeof(int));
  std::memcpy(equipmentData.data() + 24, &equipmentHeader.equipmentBasicElementSize, sizeof(int));

  return equipmentData;
}

void readPackets(char *buffer, moodycamel::ReaderWriterQueue<std::pair<int, int>> *queue) {
  int eventID = 0;
  int expectedSeqCounter = 0;
  std::vector<char> fragments;
  while (true) {
    std::pair<int, int> dataPosition;
    if (queue->try_dequeue(dataPosition)) {
      printf("2-position: %d, %d\n", dataPosition.first, dataPosition.second);
      int position = dataPosition.first;
      int size = dataPosition.second;
      if (size == 4) {
        int end_word;
        std::memcpy(&end_word, buffer + position, sizeof(int));
        if (end_word == 0xfafafafa) {
          printf("end word\n");
          expectedSeqCounter = 0;
          eventID++;
          std::vector<char> equipmentData = addEquipmentHeader(&fragments, fragments.size());
          // print equipment data
          printf("data with header:\n");
          for (int i = 0; i < equipmentData.size(); i++) {
            printf("%02x ", equipmentData[i]);
          }
          printf("\n");
          fragments.clear();
        }
      } else {
        int seq_counter;
        std::memcpy(&seq_counter, buffer + position, sizeof(int));
        printf("seq_counter: %d\n", seq_counter);
        if (seq_counter != expectedSeqCounter) {
          printf("event: %d\n", eventID);
          printf("Error! Packet mismatch, expected sequence counter %d, found %d\n",
                 expectedSeqCounter, seq_counter);
        }

        // Copy the packet to the fragments vector
        fragments.insert(fragments.end(), buffer + position, buffer + position + size);
        printf("fragments size: %d\n", fragments.size());

        // print fragments
        for (int i = 0; i < fragments.size(); i++) {
          printf("%02x ", fragments[i]);
        }

        expectedSeqCounter++;
      }
      // for (int i = dataPosition.first; i < dataPosition.first + dataPosition.second; i++) {
      //   printf("%02x ", buffer[i]);
      // }
      // std::cout << std::endl;
    }
  }
}

void readEquipment(char *buffer, moodycamel::ReaderWriterQueue<std::pair<int, int>> *queue) {
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
      std::pair<int, int> dataPosition;
      dataPosition.first = position;
      dataPosition.second = n;
      queue->enqueue(dataPosition);

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