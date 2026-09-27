#include "stm32_uart_cobs_packet.h"
#include <stdint.h>

static uint8_t decoded_buf[CP_MAX_SEND_DATA_SIZE];

/**
 * @brief パケットのヘッダー受信割り込みの予約
 * 
 * @param huart 該当 UART_HandleTypeDef のポインタ 
 * @retval 1: success
 * @retval 0: failed
 */
int cp_uart_interrupt_receive_header(UART_HandleTypeDef *huart, RecvPacket_t *recv_packet) {
  int status = HAL_UART_Receive_IT(huart, recv_packet->buf, CP_HEADER_SIZE);
  if (status != HAL_OK) {
    return 0;
  }

  return 1;
}

/**
 * @brief パケットの受信を行う
 * 
 * @param huart 該当 UART_HandleTypeDef のポインタ 
 * @param recv_packet 受信したパケットの情報を格納する RecvPacket_t 変数のポインタ
 * @retval 1: 正しい形式のパケットを受信した
 * @retval 0: パケットの受信に失敗
 */
int cp_uart_receive_payload(UART_HandleTypeDef *huart, RecvPacket_t *recv_packet) {
  int status = HAL_UART_Receive(huart, recv_packet->payload, recv_packet->header->payload_len, 1000);
  if (status == HAL_OK) { 
    return 1;
  }
  else {
    return 0;
  }
}

/**
 * @brief 受信パケットからデータを取得する
 * 
 * @param data データを格納する変数のポインタ
 * @param size 取得するデータのサイズ
 * @retval > 0: 取得したデータのサイズ
 * @retval 0: データの取得に失敗
 */
int cp_uart_get_received_data(RecvPacket_t *recv_packet, void *data, uint8_t size) {
  int ret = cobs_decode(recv_packet->payload, recv_packet->payload_len, decoded_buf, size);
  if (ret != size) {
    return 0;
  }

  uint8_t *data_p = (uint8_t *)data;
  for (int i = 0; i < size; i++) {
    data_p[i] = decoded_buf[i];
  }

  return size;
}

/**
 * @brief データをパケット化して送信
 * 
 * @param huart 該当 UART_HandleTypeDef のポインタ 
 * @param type_id パケットに付与する type_id
 * @param data 送信するデータのポインタ
 * @param size 送信するデータのサイズ
 * @retval > 0: 送信したパケットのサイズ
 * @retval 0: 送信に失敗
 */
int cp_uart_send_packet(UART_HandleTypeDef *huart, SendPacket_t *send_packet) {
  int ret = HAL_UART_Transmit(huart, send_packet->buf, send_packet->packet_size, 1000);
  if (ret != HAL_OK) {
    return 0;
  }

  return 1;
}