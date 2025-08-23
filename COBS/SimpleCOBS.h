#ifndef SIMPLE_COBS_H
#define SIMPLE_COBS_H

#include <stdint.h>

// 扱うことができる元データの最大サイズ
#define MAX_DATA_SIZE 253
// エンコード後のデータサイズ
#define COBS_ENCODED_SIZE(size) (size + 2)
#define COBS_DECODED_SIZE(size) (size - 2)

int cobs_encode(const void *data, uint8_t *encode_buf, uint8_t data_size);
int cobs_decode(const uint8_t *encoded_buf, void *data, uint8_t data_size);

#endif // SIMPLE_COBS_H
