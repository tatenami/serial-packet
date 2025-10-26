#include "../cobs_packet.h"
#include "../linux/USBSerial.hpp"
#include <cstdio>
#include <iostream>
#include <chrono>

using namespace std;

#define BUF_SIZE 10

void print_buf(const uint8_t *buf, int size) {
  for (int i = 0; i < size; i++) {
    printf("%#0x ", buf[i]);
  }
  printf("\n");
}

void print_packet(uint8_t *packet, int size) {
  PacketInfo_t info;
  cp_get_packet_info(packet, &info);

  printf("[INFO] paylaod-size: %d, checksum: %#0x\n   ", info.payload_size, info.checksum);
  print_buf(packet, size);
}

int recv_packet(USBSerial& usb, int size) {
  if (size <= HEADER_SIZE) {
    printf("[ERROR] recv buffer size is < HEADER\n");
  }

  PacketInfo_t info;

  int rsize = usb.receive(cp_recv_packet.buf, HEADER_SIZE);

  cp_get_packet_info(cp_recv_packet.buf, &info);

  rsize = usb.receive(cp_recv_packet.payload, info.payload_size);
  
  print_packet(cp_recv_packet.buf, PACKET_SIZE(sizeof(uint32_t)));
  if (rsize != info.payload_size) {
    printf("[ERROR] cannot recv packet payload\n");
    print_buf(cp_recv_packet.buf, rsize);
    return 0;
  }

  if (cp_verify_packet_checksum(cp_recv_packet.buf)) {
    printf("[ERROR] invalid checksum\n");
    print_buf(cp_recv_packet.buf, rsize);
    return 0;
  }

  return rsize;
}

int main() {
  PacketInfo_t info;
  USBSerial stm("/dev/ttyACM0");

  float theta = 0.0f;

  while (stm.available()) {

    int size = cp_make_packet(&theta, sizeof(float));

    auto start = std::chrono::high_resolution_clock::now();

    stm.send(cp_send_packet.buf, PACKET_SIZE(sizeof(float)));

    printf("send packet (%d byte)\n", size);
    print_packet(cp_send_packet.buf, size);

    float result;
    size = recv_packet(stm, PACKET_SIZE(sizeof(float)));

    auto end = std::chrono::high_resolution_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    printf("[INFO] theta: %f\n", theta);
    printf("[INFO] duration %d us\n", us);
    printf("[INFO] %f byte/s\n\n", ((float)sizeof(uint8_t) * 4 / us) * 1000000);

    size = cp_get_data(&result, sizeof(uint32_t));

    if (size > 0) {
      printf("response data: %f\n", result);
    }
    else {
      printf("[ERROR] fail to get data from packet\n");
    }

    theta += 0.1;
    usleep(500000);
  }
  
  return 0;
}