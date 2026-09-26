#ifndef SIMPLE_COBS_H
#define SIMPLE_COBS_H

#include <stdint.h>

#define COBS_ENCODED_SIZE_MAX(size) \
  ((uint32_t)(size) + ((uint32_t)(size) / 254U) + 2U)
// +2: first offset, end byte

#define COBS_DECODED_SIZE_MAX(size) ((uint32_t)(size) - 1U) 
#define COBS_DECODED_SIZE_MIN(size) ((uint32_t)(size) - 2U) 

int cobs_encode(const uint8_t *raw, const uint32_t raw_len, uint8_t *encoded_buf, uint32_t encoded_buf_size);
int cobs_decode(const uint8_t *encoded_buf, const uint32_t encoded_buf_len, uint8_t *raw, uint32_t raw_size);

#endif // SIMPLE_COBS_H
