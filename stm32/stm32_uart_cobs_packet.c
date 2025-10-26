#include "stm32_uart_cobs_packet.h"

static PacketInfo_t internal_info;
static uint8_t decoded_buf[MAX_SEND_DATA_SIZE];

/**
 * @brief 初めの受信パケット割り込みの予約
 * 
 * @param huart 該当 UART_HandleTypeDef のポインタ 
 * @retval 1: success
 * @retval 0: failed
 */
int cp_uart_init_interrupt(UART_HandleTypeDef *huart) {
  // ヘッダの受信割り込みを予約
  int status = HAL_UART_Receive_IT(huart, cp_recv_packet.buf, HEADER_SIZE);
  if (status == HAL_OK)
    return 1;
  else 
    return 0;
}

/**
 * @brief パケットの受信を行う (uart 受信割り込み関数内で実行)
 * 
 * @param huart 該当 UART_HandleTypeDef のポインタ 
 * @param info 受信したパケットの情報を格納する PacketInfo_t 変数のポインタ
 * @retval 1: 正しい形式のパケットを受信した
 * @retval 0: パケットの受信に失敗
 */
int cp_uart_interrupt_receive(UART_HandleTypeDef *huart, PacketInfo_t *info) {
  cp_get_packet_info(cp_recv_packet.buf, &internal_info);

  int status = HAL_UART_Receive(huart, cp_recv_packet.payload, internal_info.payload_size, 1000);
  if (status != HAL_OK) 
    return 0;
  
  if (cp_check_payload(cp_recv_packet.payload, internal_info.payload_size)) {
    // 次のパケットのヘッダ受信割り込み予約
    HAL_UART_Receive_IT(huart, cp_recv_packet.buf, HEADER_SIZE);
    *info = internal_info;
    return 1;
  }
  
  return 0;
}

/**
 * @brief 受信パケットからデータを取得する
 * 
 * @param data データを格納する変数のポインタ
 * @param size 取得するデータのサイズ
 * @retval > 0: 取得したデータのサイズ
 * @retval 0: データの取得に失敗
 */
int cp_uart_get_received_data(void *data, uint8_t size) {
  int ret = cobs_decode(cp_recv_packet.payload, decoded_buf, size);
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
int cp_uart_send_packet(UART_HandleTypeDef *huart, void *data, uint8_t size) {
  int exp_size = PACKET_SIZE(size);
  int ret = cp_make_packet(data, size);
  if (ret != exp_size) {
    return 0;
  }

  ret = HAL_UART_Transmit(huart, cp_send_packet.buf, exp_size, 1000);
  if (ret != HAL_OK) {
    return 0;
  }

  return exp_size;
}