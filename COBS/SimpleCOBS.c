#include "SimpleCOBS.h"

/**
 * @brief \c data をエンコードし \c encode_buf へ格納
 * 
 * @param data エンコードするデータのポインタ
 * @param encode_buf エンコードしたデータの格納先バッファのポインタ
 * @param data_size エンコードするデータのバイト数
 * @return int: エンコード後のサイズ
 */
int cobs_encode(const void *data, 
                uint8_t    *encode_buf, 
                uint8_t     data_size)
{
  const uint8_t *data_p = (const uint8_t *)data;
  uint8_t  read_index  = 0;
  uint16_t write_index = 1;
  uint8_t  mark_index  = 0;
  uint8_t  mark_offset = 1;

  if (data_size > COBS_MAX_DATA_SIZE)
    return 0;

  while (read_index < data_size) {
    if (data_p[read_index] == 0x00) {
      encode_buf[mark_index] = mark_offset;
      mark_index  = write_index;
      mark_offset = 1;
    }
    else {
      encode_buf[write_index] = data_p[read_index];
      mark_offset++;
    }

    read_index++;
    write_index++;
  }

  encode_buf[mark_index] = mark_offset;
  encode_buf[write_index] = 0x00;
  return write_index + 1;
}

/**
 * @brief \c encoded_buf をデコードし \c data へ格納
 * 
 * @param encoded_buf エンコードされたデータを格納しているバッファのポインタ
 * @param data デコードしたデータの格納先ポインタ
 * @param data_size デコード後のデータのバイト数
 * @retval > 0: デコード後のサイズ
 * @retval == 0: \c data_size がデコード後のサイズより小さい
 */
int cobs_decode(const uint8_t *encoded_buf,
                void     *data, 
                uint8_t   data_size)
{
  uint8_t *data_p = (uint8_t *)data;
  uint16_t read_index = 0;
  uint16_t cobs_size = COBS_ENCODED_SIZE(data_size);

  // 先頭・末端のバイト以外をコピー
  for (int i = 0; i < data_size; i++) {
    if (encoded_buf[i + 1] == 0x00) {
      return 0;
    }
    data_p[i] = encoded_buf[i + 1];
  }

  while (1) {
    read_index += encoded_buf[read_index];
    if (encoded_buf[read_index] == 0x00) {
      break;
    }

    if (read_index >= cobs_size) {
      return 0;
    }

    data_p[read_index - 1] = 0x00;
  } 

  if (COBS_DECODED_SIZE(read_index + 1) == data_size) {
    return data_size;
  }
  else {
    return 0;
  }
}