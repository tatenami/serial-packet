#include "cobs_packet.h"
#include "simple_cobs.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

static uint16_t FLETCHER16_BLOCK_LEN = 64;

/**
 * @brief ペイロード末尾が 0x00 かを確認
 * 
 * @param paylaod ペイロードのポインタ
 * @param len ペイロード長
 * @retval 1: ペイロード末尾 (len バイト目) が 0x00 
 * @retval 0: ペイロード末尾 (len バイト目) が 0x00 でない
 */
int cp_check_payload_encoded(const uint8_t *paylaod, uint32_t len) {
  return (paylaod[len - 1] == 0x00);
}

// ペイロードのチェックサム計算 (16 Bit)
static uint16_t calc_checksum(const uint8_t *payload, const uint16_t size) {
  uint32_t checksum = 0;
  for (int i = 0; i < size; i++) {
    checksum += payload[i];
  }

  return (uint16_t)(checksum & 0xFFFF);
}

static uint16_t calc_checksum_fletcher16(const uint8_t *payload, const uint16_t size) {
  uint32_t sum1 = 0;
  uint32_t sum2 = 0;
  uint16_t len = size;

  while (len > 0) {
    uint16_t block_len = 
      (len > FLETCHER16_BLOCK_LEN)? FLETCHER16_BLOCK_LEN : len;
    
    for (uint16_t i = 0; i < block_len; i++) {
      sum1 += payload[i];
      sum2 += sum1;
    }

    // それぞれ 8 Bit に丸める
    sum1 %= 255;
    sum2 %= 255;

    payload += block_len;
    len -= block_len;
  }

  return (uint16_t)((sum2 << 8) | sum1);
}

static void cp_make_header(uint8_t *buf, uint8_t type, uint8_t seq, 
                          uint16_t payload_len, uint16_t checksum) 
{
  buf[0] = SOP;
  buf[1] = type;
  buf[2] = seq;
  buf[3] = (payload_len >> 8) & 0xFF;
  buf[4] = payload_len & 0xFF;
  buf[5] = (checksum >> 8) & 0xFF;
  buf[6] = checksum & 0xFF;
}

/**
 * @brief SendPacket_t の初期化 (パケット割り当て)
 * 
 * @param send_packet SendPacket_t のポインタ
 * @param buf 割り当てるバッファのポインタ
 * @param buf_size 割り当てるバッファのサイズ
 * @retval 0: バッファサイズ不足
 * @retval 1: 初期化成功
 */
cp_status_t cp_init_send_packet(SendPacket_t *send_packet, uint8_t *buf, uint32_t buf_size) {
  if (buf_size < CP_HEADER_SIZE) {
    return CP_ERR_BUF_SIZE_TOO_SMALL;
  }

  send_packet->buf = buf;
  send_packet->buf_size = buf_size;
  send_packet->packet_size = 0;

  return 1;
}


/**
 * @brief データがCOBSエンコードされたパケットを作成する
 * 
 * @param packet Packet_t ポインタ
 * @param type パケットの種別値
 * @param seq パケットのシーケンス番号
 * @param data エンコードし，ペイロードにするデータ
 * @param data_size データサイズ 
 * @retval 0: パケット作成失敗
 * @retval > 0: 作成したパケットのサイズ 
 */
cp_status_t cp_make_send_packet(SendPacket_t *send_packet, uint8_t type, uint8_t seq, 
  const void *data, const uint16_t data_size)
{
  if (data_size > CP_MAX_SEND_DATA_SIZE) {
    send_packet->packet_size = 0;
    return CP_ERR_DATA_SIZE_TOO_LARGE;
  }
  
  if (send_packet->buf == NULL) {
    send_packet->packet_size = 0;
    return CP_ERR_NO_ATACHED_BUF;
  }

  uint8_t *payload = (CP_HEADER_SIZE + send_packet->buf);
  // データを COBS エンコード
  int encoded_len = cobs_encode((uint8_t *)data, data_size, 
    payload, (send_packet->buf_size - CP_HEADER_SIZE));

  if (encoded_len < 0) {
    send_packet->packet_size = 0;
    return CP_ERR_FAIL_COBS_ENCODE;
  }

  uint16_t checksum = 0;
  if (data_size > 0) {
    checksum = calc_checksum_fletcher16(payload, encoded_len);
  }

  cp_make_header(send_packet->buf, type, seq, encoded_len, checksum);
  // ヘッダ情報を SendPacket_t に格納
  send_packet->header.sop = SOP;
  send_packet->header.type = type;
  send_packet->header.seq = seq;
  send_packet->header.payload_len = encoded_len;
  send_packet->header.checksum = checksum;  

  uint32_t packet_size = (CP_HEADER_SIZE + encoded_len);
  send_packet->packet_size = packet_size;

  return packet_size;
}

/**
 * @brief 生データを解析して PacketHeader_t にデータを格納

 * @param header PacketHeader_t ポインタ
 * @param buf パケットの生データのバッファ
 * @param buf_size バッファのサイズ
 * @retval 1: 解析，データ格納成功
 * @retval < 0: バッファサイズ不足
 */
cp_status_t cp_parse_header(PacketHeader_t* const header, const uint8_t *buf, uint32_t buf_size) {
  if (buf_size < CP_HEADER_SIZE) {
    return CP_ERR_BUF_SIZE_TOO_SMALL;
  }
  
  header->sop  = buf[0];
  header->type = buf[1];
  header->seq  = buf[2];
  header->payload_len = (buf[3] << 8) | buf[4];
  header->checksum = (buf[5] << 8) | buf[6];

  return 1;
}

/**
 * @brief RecvPacket_t の初期化 (バッファ割り当て)

 * @param r_packet RecvPacket_t ポインタ
 * @param buf バッファ
 * @param buf_size バッファのサイズ
 * @retval 1: 成功
 * @retval < 0: バッファサイズ不足
 */
cp_status_t cp_init_recv_packet(RecvPacket_t *recv_packet, PacketHeader_t *header, uint8_t *buf, uint32_t buf_size) {
  if (buf_size < CP_HEADER_SIZE) {
    return CP_ERR_BUF_SIZE_TOO_SMALL;
  }

  recv_packet->header = header;
  recv_packet->buf = buf;
  recv_packet->buf_size = buf_size;
  recv_packet->payload_len = 0;

  if (buf_size == CP_HEADER_SIZE) {
    recv_packet->payload = NULL;
  }
  else {
    recv_packet->payload = (CP_HEADER_SIZE + recv_packet->buf);
  }

  return 1;
}

/**
 * @brief パケットのチェックサム検証
 * 
 * @param header 解析対象のパケットのヘッダ情報を持つ PacketHeader_t のポインタ
 * @param payload パケットペイロードのバッファのポインタ
 * @param payload_len パケットペイロードのバッファの長さ
 * @retval 1: チェックサムが一致
 * @retval 0: チェックサムが不一致
 */
int cp_verify_checksum(PacketHeader_t *header, const uint8_t *payload, uint32_t payload_len) {
  uint16_t checksum = calc_checksum_fletcher16(payload, payload_len);
  return (checksum == header->checksum);
}

cp_status_t cp_parse_recv_header(RecvPacket_t *recv_packet) {
  cp_status_t ret = cp_parse_header(recv_packet->header, recv_packet->buf, recv_packet->buf_size);
  if (ret < 0) {
    return ret;
  }

  uint32_t payload_len = recv_packet->header->payload_len;
  if (payload_len > (recv_packet->buf_size - CP_HEADER_SIZE)) {
    return CP_ERR_PAYLOAD_SIZE_TOO_LARGE;
  }

  recv_packet->payload_len = payload_len;

  return 1;
}

/**
 * @brief パケットを解析して RecvPacket_t にペイロードデータを格納
 * @param packet 受信パケットのバッファを持つ Packet_t のポインタ
 * @retval 1: パケットのサイズ
 * @retval < 0: 解析，データ格納失敗
 */
cp_status_t cp_parse_recv_packet(RecvPacket_t* recv_packet) {
  uint32_t payload_len = recv_packet->header->payload_len;

  if (payload_len == 0) {
    return CP_ERR_ZERO_PAYLOAD_LEN;
  }

  if (!cp_check_payload_encoded(recv_packet->payload, payload_len)) {
    return CP_ERR_NOT_EXIST_DELIMITER;
  }

  if (!cp_verify_checksum(recv_packet->header, recv_packet->payload, payload_len)) {
    return CP_ERR_NOT_MATCH_CHECKSUM;
  }

  return (CP_HEADER_SIZE + payload_len);
}

/**
 * @brief パケットのペイロードをデコードし，データを取得する
 * 
 * @param packet パケットのバッファ
 * @param data 格納先データ
 * @param data_size 格納先のデータのサイズ
 * @retval 0 >=: デコードしたデータのサイズ
 * @retval < 0: 失敗
 */
cp_status_t cp_get_payload_data(RecvPacket_t *recv_packet, void *buf, uint16_t buf_size) {
  if (recv_packet->payload_len == 0) {
    return CP_ERR_ZERO_PAYLOAD_LEN;
  }

  if (buf_size < COBS_MAX_DECODED_SIZE(recv_packet->payload_len)) {
    return CP_ERR_BUF_SIZE_TOO_SMALL;
  }

  int decoded_len = cobs_decode(recv_packet->payload, recv_packet->payload_len,
    (uint8_t *)buf, buf_size);

  return (decoded_len >= 0)? decoded_len : CP_ERR_FAIL_COBS_DECODE;
}
