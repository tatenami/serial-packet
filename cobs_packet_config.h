#ifndef COBS_PACKET_CONFIG_H
#define COBS_PACKET_CONFIG_H

#include "cobs_packet.h"

// デフォルト: 253 byte (パケットで扱える最大データ長)
#define MAX_DATA_SIZE_DEFAULT COBS_MAX_DATA_SIZE

#define MAX_SEND_DATA_SIZE MAX_DATA_SIZE_DEFAULT
#define MAX_RECV_DATA_SIZE MAX_DATA_SIZE_DEFAULT

#endif
