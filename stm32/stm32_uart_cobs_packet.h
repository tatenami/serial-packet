#ifndef STM32_UART_COBS_PACKET_H
#define STM32_UART_COBS_PACKET_H

#include "main.h"
#include "cobs_packet.h"

int cp_uart_init_interrupt(UART_HandleTypeDef *huart);
int cp_uart_interrupt_receive(UART_HandleTypeDef *huart, PacketInfo_t *info);
int cp_uart_get_received_data(void *data, uint8_t size);
int cp_uart_send_packet(UART_HandleTypeDef *huart, void *data, uint8_t size);

#endif // STM32_UART_COBS_PACKET_H