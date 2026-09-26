#ifndef COBS_PACKET_H
#define COBS_PACKET_H

#include <stdint.h>
#include "COBS/SimpleCOBS.h"
#include "cobs_packet_config.h"

// start of packet
#define SOP  (0xAA) 
// header (7 byte): | SOP(1) | type(1) | seq(1) | payload_size(2) | check-sum(2) |
#define CP_HEADER_SIZE (7)
#define CP_PAYLOAD_SIZE_MAX COBS_ENCODED_SIZE_MAX(CP_MAX_DATA_SIZE)
#define CP_PACKET_SIZE_MAX(data_size) (CP_HEADER_SIZE + COBS_ENCODED_SIZE_MAX(data_size))

typedef struct __attribute__((packed)) {
  uint8_t  sop;  // パケット先頭検出用固定値
  uint8_t  type; // パケット種別 
  uint8_t  seq;  // シーケンス番号 
  uint16_t payload_len; // ペイロード長 (COBS エンコード後)
  uint16_t checksum; // ペイロードのチェックサム
} PacketHeader_t;

typedef struct {
  uint8_t *buf;
  uint32_t buf_size;
} SendPacket_t;

typedef struct {
  uint8_t *buf; 
  uint32_t buf_size;
  uint8_t *payload; // バッファアドレス + HEADER_SIZE
  uint32_t payload_len;
} RecvPacket_t;

int cp_check_payload_encoded(const uint8_t *paylaod, uint32_t size);
int cp_init_send_packet(SendPacket_t *send_packet, uint8_t *buf, uint32_t buf_size);
int cp_make_send_packet(SendPacket_t *send_packet, uint8_t type, uint8_t seq, const void *data, const uint16_t data_size);
int cp_parse_header(PacketHeader_t* const header, const uint8_t *buf, uint32_t buf_size);
int cp_init_recv_packet(RecvPacket_t *recv_packet, uint8_t *buf, uint32_t buf_size);
int cp_verify_checksum(PacketHeader_t *header, const uint8_t *payload, uint32_t payload_len);
int cp_parse_recv_packet(PacketHeader_t *header, RecvPacket_t* const recv_packet);
int cp_get_payload_data(RecvPacket_t *recv_packet, void *data, uint16_t data_size);

#endif // PACKET_H