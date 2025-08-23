#include "packet.h"

int parse_header(uint8_t *data, pHeader_t *header) {
  if (data[0] != HEADER_VAL) return 0;

  header->header = data[0];
  header->type_id = data[1];
  header->size = data[2];

  return 1;
}