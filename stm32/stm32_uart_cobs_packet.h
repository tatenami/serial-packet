#ifndef STM32_UART_COBS_PACKET_H
#define STM32_UART_COBS_PACKET_H

#include "main.h"
#include "cobs_packet.h"

// "ucp" is prefix that mean "uart cobs packet"

/**
 * @brief 初めの受信パケット割り込みの予約
 * 
 * @param huart 該当 UART_HandleTypeDef のポインタ 
 * @retval 1: success
 * @retval 0: failed
 */
int ucp_init_interrupt(UART_HandleTypeDef *huart);

/**
 * @brief パケットの受信を行う (uart 受信割り込み関数内で実行すること)
 * 
 * @param huart 該当 UART_HandleTypeDef のポインタ 
 * @param info 受信したパケットの情報を格納する PacketInfo_t 変数のポインタ
 * @retval 1: 正しい形式のパケットを受信した
 * @retval 0: パケットの受信に失敗
 */
int ucp_interrupt_receive(UART_HandleTypeDef *huart, PacketInfo_t *info);

/**
 * @brief 受信パケットからデータを取得する
 * 
 * @param data データを格納する変数のポインタ
 * @param size 取得するデータのサイズ
 * @retval > 0: 取得したデータのサイズ
 * @retval 0: データの取得に失敗
 */
int ucp_get_received_data(void *data, uint8_t size);

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
int ucp_send_packet(UART_HandleTypeDef *huart, void *data, uint8_t size);

#endif // STM32_UART_COBS_PACKET_H