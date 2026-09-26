#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "../cobs_packet.h"

#define TEST_BUF_SIZE CP_PACKET_SIZE_MAX(256)

static void dump_hex(const char *label, const uint8_t *buf, size_t len) {
    printf("%s (%zu): ", label, len);
    for (size_t i = 0; i < len; i++) {
        printf("%02X ", buf[i]);
    }
    printf("\n");
}

static void print_header(PacketHeader_t *header) {
  printf("[header]\nSOP:%04X\nTYPE:%d\nSEQ:%d\nPLEN:%d\nCSUM:%d\n",
    header->sop, header->type, header->seq, header->payload_len, header->checksum);
}

int main(void) {
    // ===== 元データ =====
    uint8_t original_data[] = {
        0x11, 0x22, 0x00, 0x33, 0x44, 0x55
    };
    size_t original_size = sizeof(original_data);

    dump_hex("original", original_data, original_size);

    // ===== 送信用バッファ =====
    uint8_t send_buf[TEST_BUF_SIZE] = {0};

    SendPacket_t s_packet;
    if (!cp_init_send_packet(&s_packet, send_buf, TEST_BUF_SIZE)) {
      printf("[ERROR] init send packet failed\n");
    }

    // ===== パケット生成 =====
    int packet_size = cp_make_send_packet(
        &s_packet,
        0x01,      // type
        0x10,      // seq
        original_data,
        original_size
    );

    // send_packet.payload[2] ^= 0xFF;

    assert(packet_size > 0);

    printf("=== Packet Generated ===\n");
    dump_hex("packet", s_packet.buf, packet_size);

    // ===== 受信側パース =====
    PacketHeader_t header;
    RecvPacket_t recv_packet = {0};

    cp_init_recv_packet(&recv_packet, send_buf, TEST_BUF_SIZE);

    cp_parse_header(&header, recv_packet.buf, recv_packet.buf_size);
     
    int parse_result = cp_parse_recv_packet(
      &header,
      &recv_packet
    );

    assert(parse_result == 1);

    printf("=== Packet Parsed ===\n");

    dump_hex("recv packet", recv_packet.buf, packet_size);

    // ===== チェックサム検証 =====
    int checksum_ok = cp_verify_checksum(&header, recv_packet.payload, recv_packet.payload_len);
    assert(checksum_ok == 1);

    // ===== デコード =====
    uint8_t decoded_data[256] = {0};

    int decoded_size = cp_get_payload_data(
        &recv_packet,
        decoded_data,
        sizeof(decoded_data)
    );

    printf("decoded size: %d\n", decoded_size);

    assert(decoded_size >= 0);

    printf("=== Decoded Data ===\n");
    dump_hex("decoded", decoded_data, decoded_size);

    // ===== 一致確認 =====
    assert(decoded_size == original_size);
    assert(memcmp(original_data, decoded_data, original_size) == 0);

    printf("=== TEST PASSED ===\n");

    return 0;
}