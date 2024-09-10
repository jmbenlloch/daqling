#include "event.h"
#include "readerwriterqueue.h"
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <chrono>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

#define PORT 6123
#define BUFFERSIZE 950000

struct equipmentDataType {
  int eventID;
  int equipmentID;
  std::vector<char> data;
};

void readEquipment(char *buffer, moodycamel::ReaderWriterQueue<std::pair<int, int>> *queue);
void readPackets(char *buffer, moodycamel::ReaderWriterQueue<std::pair<int, int>> *queue,
                 moodycamel::ReaderWriterQueue<equipmentDataType> *readyFragments);
void readSubEvents(moodycamel::ReaderWriterQueue<equipmentDataType> *readyFragments);
std::vector<char> addLDCHeader(std::map<int, std::vector<char>> equipmentsData, int eventID,
                               int runNumber);

int main() {
  moodycamel::ReaderWriterQueue<std::pair<int, int>> q(100);
  moodycamel::ReaderWriterQueue<equipmentDataType> readyFragments(100);
  char buffer[BUFFERSIZE];
  std::thread t1(readEquipment, buffer, &q);
  std::thread t2(readPackets, buffer, &q, &readyFragments);
  std::thread t3(readSubEvents, &readyFragments);
  t1.join();
  return 0;
}

void readSubEvents(moodycamel::ReaderWriterQueue<equipmentDataType> *readyFragments) {
  std::map<int, std::map<int, std::vector<char>>> subevents;

  equipmentDataType subevent;
  while (true) {
    if (readyFragments->try_dequeue(subevent)) {
      printf("subevent reader: size: %d\n", subevent.data.size());
      for (int i = 0; i < subevent.data.size(); i++) {
        printf("%02x ", subevent.data[i]);
      }
      printf("\n");

      // check subevent map contains the eventID
      if (subevents.find(subevent.eventID) == subevents.end()) {
        std::map<int, std::vector<char>> equipmentMap;
        subevents[subevent.eventID] = equipmentMap;
      }
      subevents[subevent.eventID][subevent.equipmentID] = subevent.data;

      // check if all equipment data is present
      int nEquipments = 1;
      if (subevents[subevent.eventID].size() == nEquipments) {
        printf("eventID: %d\n", subevent.eventID);
        printf("equipmentID: %d\n", subevent.equipmentID);

        std::vector<char> ldcData =
            addLDCHeader(subevents[subevent.eventID], subevent.eventID, 12345789);
        printf("ldcdata: \n");
        for (int i = 0; i < ldcData.size(); i++) {
          printf("%02x ", ldcData[i]);
        }
        printf("\n");
      }
    }
  }
}

std::vector<char> addLDCHeader(std::map<int, std::vector<char>> equipmentsData, int eventID,
                               int runNumber) {
  int ldcID = 1;

  // Get the current time from the system clock
  auto now = std::chrono::system_clock::now();
  auto duration = now.time_since_epoch();
  auto timestampSec = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
  auto timestampUSec = std::chrono::duration_cast<std::chrono::seconds>(duration).count();

  // Calculate the total size of the equipment data
  int totalSizeEquipmentData = 0;
  for (const auto &[key, value] : equipmentsData) {
    totalSizeEquipmentData += value.size();
  }

  // Build LDC header
  eventHeaderStruct ldcHeader{
      .eventSize = eventSizeType(totalSizeEquipmentData + 20 * 4),
      .eventMagic = eventMagicType(EVENT_MAGIC_NUMBER),
      .eventHeadSize = 80,
      .eventVersion = eventVersionType(EVENT_CURRENT_VERSION),
      .eventType = eventTypeType(PHYSICS_EVENT),
      .eventRunNb = eventRunNbType(runNumber),
      .eventId = {eventID, 0},
      .eventTypeAttribute = {0, 0, ORIGINAL_EVENT},
      .eventLdcId = eventLdcIdType(ldcID),
      .eventGdcId = eventGdcIdType(GDC_VOID),
      .eventTimestampSec = eventTimestampSecType(timestampSec),
      .eventTimestampUsec = eventTimestampUsecType(timestampUSec),
  };

  std::vector<char> ldcData(totalSizeEquipmentData + 20 * 4);

  int position = 80;
  for (const auto &[key, data] : equipmentsData) {
    int size = data.size();
    std::memcpy(ldcData.data() + position, data.data(), size);
    position += size;
  }

  std::memcpy(ldcData.data(), &ldcHeader.eventSize, sizeof(int));
  std::memcpy(ldcData.data() + 4, &ldcHeader.eventMagic, sizeof(int));
  std::memcpy(ldcData.data() + 8, &ldcHeader.eventHeadSize, sizeof(int));
  std::memcpy(ldcData.data() + 12, &ldcHeader.eventVersion, sizeof(int));
  std::memcpy(ldcData.data() + 16, &ldcHeader.eventType, sizeof(int));
  std::memcpy(ldcData.data() + 20, &ldcHeader.eventRunNb, sizeof(int));
  std::memcpy(ldcData.data() + 24, &ldcHeader.eventId[0], sizeof(int));
  std::memcpy(ldcData.data() + 60, &ldcHeader.eventTypeAttribute[2], sizeof(int));
  std::memcpy(ldcData.data() + 64, &ldcHeader.eventLdcId, sizeof(int));
  std::memcpy(ldcData.data() + 68, &ldcHeader.eventGdcId, sizeof(int));
  std::memcpy(ldcData.data() + 72, &ldcHeader.eventTimestampSec, sizeof(int));
  std::memcpy(ldcData.data() + 76, &ldcHeader.eventTimestampUsec, sizeof(int));

  return ldcData;
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

void readPackets(char *buffer, moodycamel::ReaderWriterQueue<std::pair<int, int>> *queue,
                 moodycamel::ReaderWriterQueue<equipmentDataType> *readyFragments) {
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
          equipmentDataType equipmentData;
          equipmentData.eventID = eventID;
          equipmentData.equipmentID = 2;
          equipmentData.data = addEquipmentHeader(&fragments, fragments.size());
          readyFragments->enqueue(equipmentData);
          // print equipment data
          printf("data with header:\n");
          for (int i = 0; i < equipmentData.data.size(); i++) {
            printf("%02x ", equipmentData.data[i]);
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