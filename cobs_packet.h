#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>
#include "SimpleCOBS.h"
#include "cobs_packet_config.h"

#define USE_INTERNAL_BUF 1

typedef struct {
  uint8_t payload_size; // data size (encoded)
  uint8_t checksum; // check sum of payload
} PacketInfo_t;

typedef struct {
  uint8_t *const buf;
  uint8_t *const payload;
} PacketBuf_t;

// start of packet
#define SOP  (0xAA) 
// header (3byte): | HAEDER_VAL (0xAA) | payload_size | check-sum | 
#define HEADER_SIZE (3)
#define PAYLOAD_MAX_SIZE COBS_ENCODED_SIZE(COBS_MAX_DATA_SIZE)
#define PACKET_SIZE(data_size) (HEADER_SIZE + COBS_ENCODED_SIZE(data_size))
#define PACKET_MAX_SIZE (PACKET_SIZE(COBS_MAX_DATA_SIZE))

int cp_check_payload(const uint8_t *paylaod, uint8_t size);
void cp_get_packet_info(const uint8_t *packet, PacketInfo_t *info);
uint8_t cp_verify_checksum(const uint8_t *payload, const PacketInfo_t *info);
uint8_t cp_verify_packet_checksum(const uint8_t *pakcet);

#if USE_INTERNAL_BUF

extern PacketBuf_t cp_send_packet;
extern PacketBuf_t cp_recv_packet;

int cp_make_packet(const void *data, const uint8_t data_size);
int cp_get_data(void *data, uint8_t data_size);

#else

int cp_make_packet(uint8_t *packet, const void *data, const uint8_t data_size);
int cp_get_data(uint8_t *packet, void *data, uint8_t data_size);

#endif


#endif // PACKET_H