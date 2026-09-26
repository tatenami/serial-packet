#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "../COBS/SimpleCOBS.h"

#define BUF_SIZE 1024

static void dump(const char *label, const uint8_t *buf, uint32_t len)
{
    printf("%s (%u): ", label, len);
    for (uint32_t i = 0; i < len; i++) {
        printf("%02X ", buf[i]);
    }
    printf("\n");
}

static int test_case(const uint8_t *input, uint32_t len)
{
    uint8_t encoded[BUF_SIZE];
    uint8_t decoded[BUF_SIZE];

    uint32_t enc_len = 0;
    uint32_t dec_len = 0;

    int ret;

    printf("origin len: %d\n", len);
    enc_len = cobs_encode(input, len, encoded, BUF_SIZE);
    printf("encoded len: %d\n", enc_len);
    if (enc_len < 0) {
        printf("Encode error\n");
        return -1;
    }

    dec_len = cobs_decode(encoded, enc_len, decoded, BUF_SIZE);
    printf("decoded len: %d\n", dec_len);
    if (dec_len < 0) {
        printf("Decode error\n");
        dump("ENC", encoded, enc_len);
        return -1;
    }

    if (len != dec_len || memcmp(input, decoded, len) != 0) {
        printf("Mismatch!\n");
        dump("IN ", input, len);
        dump("ENC", encoded, enc_len);
        dump("DEC", decoded, dec_len);
        return -1;
    }

    return 0;
}

static void run_fixed_tests(void)
{
    uint8_t buf[512];

    // 2. ゼロなし
    uint8_t no_zero[] = {1,2,3,4,5};
    test_case(no_zero, sizeof(no_zero));

    // 3. 先頭ゼロ
    uint8_t head_zero[] = {0,1,2,3};
    test_case(head_zero, sizeof(head_zero));

    // 4. 末尾ゼロ
    uint8_t tail_zero[] = {1,2,3,0};
    test_case(tail_zero, sizeof(tail_zero));

    // 5. 連続ゼロ
    uint8_t multi_zero[] = {1,0,0,0,2};
    test_case(multi_zero, sizeof(multi_zero));

    // 6. 254バイト境界（ゼロなし）
    for (int i = 0; i < 254; i++) buf[i] = i+1;
    test_case(buf, 254);

    // 7. 255バイト（分割発生）
    for (int i = 0; i < 255; i++) buf[i] = i+1;
    test_case(buf, 255);

    // 8. 508バイト（2ブロック）
    for (int i = 0; i < 508; i++) buf[i] = (i % 250) + 1;
    test_case(buf, 508);
}

static void run_random_tests(void)
{
    uint8_t buf[512];

    for (int t = 0; t < 10000; t++) {
      printf("[random No.%d]\n", t);
        uint32_t len = rand() % 512;

        if (len == 0) {
          printf("!! len == 0 is dont convert\n");
          continue;
        }

        for (uint32_t i = 0; i < len; i++) {
            buf[i] = rand() % 256;
        }

        if (test_case(buf, len) != 0) {
            printf("Random test failed at iteration %d\n", t);
            return;
        }
    }

    printf("Random tests passed\n");
}

int main(void)
{
    srand(0);

    run_fixed_tests();
    run_random_tests();

    printf("All tests passed\n");
    return 0;
}