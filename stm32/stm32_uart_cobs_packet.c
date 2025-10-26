#include "stm32_uart_cobs_packet.h"

static PacketInfo_t internal_info;
static uint8_t decoded_buf[MAX_SEND_DATA_SIZE];

int ucp_init_interrupt(UART_HandleTypeDef *huart) {
  // ヘッダの受信割り込みを予約
  int status = HAL_UART_Receive_IT(huart, cp_recv_packet.buf, HEADER_SIZE);
  if (status == HAL_OK)
    return 1;
  else 
    return 0;
}

int ucp_interrupt_receive(UART_HandleTypeDef *huart, PacketInfo_t *info) {
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

int ucp_get_received_data(void *data, uint8_t size) {
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

int ucp_send_packet(UART_HandleTypeDef *huart, void *data, uint8_t size) {
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