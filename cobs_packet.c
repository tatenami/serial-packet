#include "cobs_packet.h"

/**
 * @brief ペイロード末尾が 0x00 かを確認
 * 
 * @param paylaod ペイロードのポインタ
 * @param size ペイロード長
 * @retval 1: ペイロード末尾 (size バイト目) が 0x00 
 * @retval 0: ペイロード末尾 (size バイト目) が 0x00 でない
 */
int cp_check_payload(const uint8_t *paylaod, uint8_t size) {
  return (paylaod[size - 1] == 0x00);
}

/**
 * @brief パケットのペイロードサイズとチェックサムを取得する
 * 
 * @param packet パケットのバッファ
 * @param info 格納先のパケット情報構造体のポインタ
 */
void cp_get_packet_info(const uint8_t *packet, PacketInfo_t *info) {
  info->payload_size = packet[1];
  info->checksum = packet[2];
}

// ペイロードのチェックサム計算 (8 Bit)
static uint8_t calc_checksum(const uint8_t *payload, const uint8_t size) {
  uint32_t checksum = 0;
  for (int i = 0; i < size; i++) {
    checksum += payload[i];
  }

  return (uint8_t)(checksum & 0xFF);
}

/**
 * @brief ヘッダとペイロードのチェックサム照合
 * 
 * @param payload ペイロードのポインタ
 * @param info PacketInfo_t のポインタ
 * @retval 1: チェックサムが一致
 * @retval 0: チェックサムが不一致
 */
uint8_t cp_verify_checksum(const uint8_t *payload, const PacketInfo_t *info) {
  uint32_t payload_checksum = calc_checksum(payload, info->payload_size);
  return (payload_checksum == info->checksum);
}

/**
 * @brief パケットのチェックサム検証
 * 
 * @param pakcet パケットのバッファ
 * @retval 1: チェックサムが一致
 * @retval 0: チェックサムが不一致
 */
uint8_t cp_verify_packet_checksum(const uint8_t *pakcet) {
  PacketInfo_t info;
  cp_get_packet_info(pakcet, &info);
  return cp_verify_checksum(pakcet, &info);
}

#if USE_INTERNAL_BUF

// 内部用パケットバッファ
static uint8_t internal_send_buf[PACKET_SIZE(MAX_SEND_DATA_SIZE)] = {};
static uint8_t internal_recv_buf[PACKET_SIZE(MAX_RECV_DATA_SIZE)] = {};

PacketBuf_t cp_send_packet = {
  .buf = internal_send_buf,
  .payload = &internal_send_buf[HEADER_SIZE],
};
PacketBuf_t cp_recv_packet = {
  .buf = internal_recv_buf,
  .payload = &internal_recv_buf[HEADER_SIZE],
};

/**
 * @brief データがCOBSエンコードされたパケットを作成する
 * 
 * @param type_id 
 * @param data エンコードし，ペイロードにするデータ
 * @param data_size データサイズ 
 * @retval 0: パケット作成失敗
 * @retval > 0: 作成したパケットのサイズ 
 */
int cp_make_packet(const void *data, const uint8_t data_size) {
  uint8_t exp_encoded_size = COBS_ENCODED_SIZE(data_size);

  internal_send_buf[0] = SOP;

  uint8_t encoded_size = cobs_encode(data, cp_send_packet.payload, data_size);
  if (exp_encoded_size != encoded_size) {
    return 0;
  }

  internal_send_buf[1] = encoded_size;
  if (cp_send_packet.payload[encoded_size - 1] != 0x00) {
    return 0;
  }

  internal_send_buf[2] = calc_checksum(cp_send_packet.payload, encoded_size);

  return PACKET_SIZE(data_size);
}

/**
 * @brief パケットのペイロードをデコードし，データを取得する
 * 
 * @param data 格納先データ
 * @param data_size 格納先のデータのサイズ
 * @retval 0: 失敗
 * @retval 0 >: デコードしたデータのサイズ
 */
int cp_get_data(void *data, uint8_t data_size) {
  uint8_t *payload = &internal_recv_buf[HEADER_SIZE];

  if (!cp_check_payload(payload, COBS_ENCODED_SIZE(data_size)))
    return 0;

  uint8_t decoded_size = cobs_decode(payload, (uint8_t *)data, data_size);
  if (decoded_size != data_size) 
    return 0;

  return data_size;
}

#else

/**
 * @brief データがCOBSエンコードされたパケットを作成する
 * 
 * @param packet パケットのバッファ
 * @param data エンコードし，ペイロードにするデータ
 * @param data_size データサイズ 
 * @retval 0: パケット作成失敗
 * @retval > 0: 作成したパケットのサイズ 
 */
int cp_make_packet(
  uint8_t *packet, 
  const void *data, 
  const uint8_t data_size)
{
  uint8_t *payload = &packet[HEADER_SIZE];
  uint8_t exp_encoded_size = COBS_ENCODED_SIZE(data_size);

  packet[0] = SOP;

  uint8_t encoded_size = cobs_encode(data, payload, data_size);
  if (exp_encoded_size != encoded_size) {
    return 0;
  }

  packet[1] = encoded_size;
  if (payload[encoded_size - 1] != 0x00) {
    return 0;
  }

  packet[2] = calc_checksum(packet, encoded_size);

  return PACKET_SIZE(data_size);
}

/**
 * @brief パケットのペイロードをデコードし，データを取得する
 * 
 * @param packet パケットのバッファ
 * @param data 格納先データ
 * @param data_size 格納先のデータのサイズ
 * @retval 0: 失敗
 * @retval 0 >: デコードしたデータのサイズ
 */
int cp_get_data(uint8_t *packet, void *data, uint8_t data_size) {
  uint8_t *payload = &packet[HEADER_SIZE];

  if (!cp_check_payload(payload, COBS_ENCODED_SIZE(data_size))) {
    return 0;
  }

  uint8_t decoded_size = cobs_decode(payload, data, data_size);
  if (decoded_size != data_size) 
    return 0;

  return data_size;
}

#endif