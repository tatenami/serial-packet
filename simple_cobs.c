#include "simple_cobs.h"

/**
 * @brief \c raw をエンコードし \c encoded_buf へ格納
 * 
 * @param raw エンコードするデータのポインタ
 * @param raw_size エンコードするデータのバイト数
 * @param encoded_buf エンコードしたデータの格納先バッファのポインタ
 * @param encoded_buf_size 
 * @retval >= 0: エンコード後のサイズ
 * @retval < 0: error
 */
int cobs_encode(const uint8_t *raw, 
                const uint32_t raw_len, 
                uint8_t       *encoded_buf,
                uint32_t       encoded_buf_size)
{
  const uint8_t *data_p = (const uint8_t *)raw;
  uint32_t read_index  = 0;
  uint32_t write_index = 1;
  uint32_t mark_index  = 0;
  uint8_t  mark_offset = 1;

  if (encoded_buf_size < COBS_ENCODED_SIZE_MAX(raw_len))
    return -1;

  while (read_index < raw_len) {
    if (data_p[read_index] == 0x00) {
      encoded_buf[mark_index] = mark_offset;
      mark_index  = write_index;
      mark_offset = 1;
    }
    else {
      encoded_buf[write_index] = data_p[read_index];
      mark_offset++;

      // offset が 255 であれば offset byte を追加
      if (mark_offset == 0xFF) {
        encoded_buf[mark_index] = mark_offset;
        mark_index  = write_index + 1;
        mark_offset = 1;
        write_index++;
      }
    }

    read_index++;
    write_index++;
  }

  encoded_buf[mark_index] = mark_offset;
  encoded_buf[write_index] = 0x00;
  
  if (raw_len > 0) {
    return write_index + 1;
  }
  else {
    return 0;
  }
}

/**
 * @brief \c encoded_buf をデコードし \c raw へ格納
 * 
 * @param encoded_buf エンコードされたデータのポインタ
 * @param encoded_buf_size エンコードされたデータのバイト数
 * @param raw デコード後のデータのポインタ
 * @param raw_size デコード後のデータのバイト数
 * @retval >= 0: デコード後のサイズ
 * @retval < 0: error
 */
int cobs_decode(const uint8_t *encoded_buf,
                const uint32_t encoded_buf_len, 
                uint8_t       *raw,
                uint32_t       raw_size)
{
  uint32_t read_index  = 0;
  uint32_t write_index = 0;

  if (encoded_buf_len == 0) {
    return -1;
  }

  if (raw_size == 0) {
    return -1;
  }

  if (encoded_buf[encoded_buf_len - 1] != 0x00) {
    return -1;
  }

  // 末端の 0x00 までの範囲を処理
  while (read_index < (encoded_buf_len - 1)) {
    uint8_t offset = encoded_buf[read_index];

    if (offset == 0){
      return -1;
    }
    if (read_index + offset > encoded_buf_len) {
      return -1;
    }

    read_index++;
    for (uint8_t i = 1; i < offset; i++) {
      if (write_index >= raw_size) {
        return -1;
      }

      raw[write_index] = encoded_buf[read_index];
      read_index++;
      write_index++;
    }

    if ((offset != 0xFF) && (read_index < (encoded_buf_len - 1))) {
      if (write_index >= raw_size) {
        return -1;
      }

      raw[write_index] = 0x00;
      write_index++;
    }
  }

  if (write_index > 0) {
    return write_index;
  }
  else {
    return 0;
  }
}