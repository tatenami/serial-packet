#include "../cobs_packet.h"
#include "../linux/USBSerial.hpp"
#include <cstdio>
#include <iostream>
#include <chrono>

using namespace std;

void print_buf(const uint8_t *buf, int size) {
  for (int i = 0; i < size; i++) {
    printf("%#0x ", buf[i]);
  }
  printf("\n");
}


int main() {
  USBSerial stm("/dev/ttyACM0");

  float theta = 0.0f;

  while (stm.available()) {

  }
  
  return 0;
}