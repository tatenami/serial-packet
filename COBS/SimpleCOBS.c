#include "SimpleCOBS.h"

/**
 * @brief \c data をエンコードし \c encode_buf へ格納
 * 
 * @param data エンコードするデータのポインタ
 * @param data_size エンコードするデータのバイト数
 * @param encode_buf エンコードしたデータの格納先バッファのポインタ
 * @return int: エンコード後のサイズ
 */
int cobs_encode(const void *data, 
                uint8_t    *encode_buf, 
                uint8_t     data_size)
{
  const uint8_t *data_p = (const uint8_t *)data;
  uint8_t read_index  = 0;
  int     write_index = 1;
  int     mark_index  = 0;
  uint8_t mark_offset = 1;

  if (data_size > MAX_DATA_SIZE)
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
 * @param data デコードしたデータの格納先ポインタ
 * @param data_size デコード後のデータのバイト数
 * @param encoded_buf エンコードされたデータを格納しているバッファのポインタ
 * @retval > 0: デコード後のサイズ
 * @retval == 0: \c data_size がデコード後のサイズより小さい or 
 */
int cobs_decode(const uint8_t *encoded_buf,
                void     *data, 
                uint8_t   data_size)
{
  uint8_t *data_p = (uint8_t *)data;
  int read_index  = 0;
  int mark_index = 0;
  int offset;
  int cobs_size = COBS_ENCODED_SIZE(data_size);

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

  if (read_index - 1 == data_size) {
    return data_size;
  }
  else {
    return 0;
  }
}