#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include "packet.h"

#define USE_DATA_TYPE_SAMPLE 0

#if USE_DATA_TYPE_SAMPLE
// パケットデータタイプのサンプル
DataType_t char_data = {0, sizeof(char)};
DataType_t i8_data   = {1, sizeof(signed char)};
DataType_t u8_data   = {2, sizeof(unsigned char)};
DataType_t i16_data  = {3, sizeof(signed short)};
DataType_t u16_data  = {4, sizeof(unsigned short)};
DataType_t i32_data  = {5, sizeof(signed int)};
DataType_t u32_data  = {6, sizeof(unsigned int)};
DataType_t float_data  = {7, sizeof(float)};
DataType_t double_data = {8, sizeof(double)};
DataType_t rgb_data = {9, (sizeof(unsigned char) * 3)};

#endif

#endif // DATA_TYPES_H