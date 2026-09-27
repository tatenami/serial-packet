#ifndef STM32_UART_COBS_PACKET_H
#define STM32_UART_COBS_PACKET_H

#include "main.h"
#include "../cobs_packet.h"

int cp_uart_interrupt_receive_header(UART_HandleTypeDef *huart, RecvPacket_t *recv_packet);
int cp_uart_receive_payload(UART_HandleTypeDef *huart, RecvPacket_t *recv_packet);
int cp_uart_get_received_data(RecvPacket_t *recv_packet, void *data, uint8_t size);
int cp_uart_send_packet(UART_HandleTypeDef *huart, SendPacket_t *send_packet);

#endif // STM32_UART_COBS_PACKET_H