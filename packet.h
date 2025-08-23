#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>

/**
 * @brief ヘッダの先頭バイトの識別用設定値
 * @details ヘッダの構造体では uint8_t で宣言しているため，0~255 である必要あり 
 * 
 */
#define HEADER_VAL 0xAA

/** 
 * @brief パケットのヘッダ部のデータをまとめた構造体
 * @note type の値は自由に定義して良い
 */
typedef struct {
  uint8_t header;  // packet header value
  uint8_t type_id; // data type id
  uint8_t size;    // data size
} pHeader_t;

/**
 * @brief パケットのデータ部に関するデータをまとめた構造体
 * @note type_id, data_size ともに自由に定義してよいが，id の重複は望ましくない
 */
typedef struct {
  uint8_t type_id;
  uint8_t data_size;
} DataType_t;

/**
 * @brief バイトデータ列をヘッダとして，ヘッダ構造体にデータを変換
 * 
 * @param data バイトデータ列へのポインタ
 * @param header ヘッダ情報をまとめた構造体: pHeader_t のポインタ
 * @retval 0: ヘッダの先頭の値が識別用設定値でない
 * @retval 1: 変換成功
 */
int parse_header(uint8_t *data, pHeader_t *header);

#endif // PACKET_H