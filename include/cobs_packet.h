#ifndef COBS_PACKET_H
#define COBS_PACKET_H

#include <stdint.h>
#include "simple_cobs.h"
#include "cobs_packet_config.h"

// start of packet
#define SOP  (0xAA) 
// header (7 byte): | SOP(1) | type(1) | seq(1) | payload_size(2) | check-sum(2) |
#define CP_HEADER_SIZE (7)
#define CP_MAX_PAYLOAD_SIZE COBS_MAX_ENCODED_SIZE(CP_MAX_DATA_SIZE)
#define CP_MAX_PACKET_SIZE(data_size) (CP_HEADER_SIZE + COBS_MAX_ENCODED_SIZE(data_size))
#define CP_MAX_SUPPORTED_PAYLOAD_SIZE UINT16_MAX // このライブラリで利用可能な最大サイズ
#define CP_MAX_SUPPORTED_DATA_SIZE COBS_MAX_DECODED_SIZE(CP_MAX_SUPPORTED_PAYLOAD_SIZE)

// 処理ステータス
typedef int32_t cp_status_t;

typedef struct __attribute__((packed)) {
  uint8_t  sop;  // パケット先頭検出用固定値
  uint8_t  type; // パケット種別 
  uint8_t  seq;  // シーケンス番号 
  uint16_t payload_len; // ペイロード長 (COBS エンコード後)
  uint16_t checksum; // ペイロードのチェックサム
} PacketHeader_t;

/**
 * @brief 送信用パケットのデータ管理
 * 
 */
typedef struct {
  uint8_t *buf;
  uint32_t buf_size;
  uint32_t packet_size;
  PacketHeader_t header;
} SendPacket_t;

/**
 * @brief 受信用パケットのデータ管理
 * 
 */
typedef struct {
  uint8_t *buf; 
  uint32_t buf_size;
  uint8_t *payload; // バッファアドレス + HEADER_SIZE
  uint32_t payload_len;
  PacketHeader_t *header;
} RecvPacket_t;

int cp_check_payload_encoded(const uint8_t *paylaod, uint16_t size);
cp_status_t cp_init_send_packet(SendPacket_t *send_packet, uint8_t *buf, uint32_t buf_size);
cp_status_t cp_make_send_packet(SendPacket_t *send_packet, uint8_t type, uint8_t seq, const void *data, const uint16_t data_size);
cp_status_t cp_parse_header(PacketHeader_t* const header, const uint8_t *buf, uint16_t buf_size);
cp_status_t cp_init_recv_packet(RecvPacket_t *recv_packet, PacketHeader_t *header, uint8_t *buf, uint32_t buf_size);
int cp_verify_checksum(PacketHeader_t *header, const uint8_t *payload, uint16_t payload_len);
cp_status_t cp_parse_recv_header(RecvPacket_t *recv_packet);
cp_status_t cp_parse_recv_packet(RecvPacket_t *recv_packet);
cp_status_t cp_get_payload_data(RecvPacket_t *recv_packet, void *buf, uint16_t buf_size);

typedef enum {
  CP_ERR_BUF_SIZE_TOO_SMALL     = -1,
  CP_ERR_DATA_SIZE_TOO_LARGE    = -2,
  CP_ERR_PAYLOAD_SIZE_TOO_LARGE = -3,
  CP_ERR_FAIL_COBS_ENCODE       = -4,
  CP_ERR_FAIL_COBS_DECODE       = -5,
  CP_ERR_NOT_MATCH_CHECKSUM     = -6,
  CP_ERR_NOT_EXIST_DELIMITER    = -7,
  CP_ERR_ZERO_PAYLOAD_LEN       = -8,
  CP_ERR_NO_ATACHED_BUF         = -9,
} cp_err_t;

#endif // PACKET_H