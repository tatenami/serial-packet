# serial-packet

C/C++ 向けの，シリアル通信でデータを送受信するための軽量なパケットライブラリです．データを COBS でエンコードし，ヘッダーとチェックサムを付けたパケットを作成・解析します．

## 特徴

- C 言語で書かれたパケット処理と COBS エンコード／デコード
- パケット種別 (`type`) とシーケンス番号 (`seq`) をヘッダーに格納
- Fletcher-16 チェックサムによるペイロード検証
- Linux 向けシリアルポート補助クラスと STM32 HAL 向け UART 補助関数
- 送受信バッファを呼び出し側で用意するため，組み込み環境でも利用可能

## パケット形式

```text
| ヘッダー (7 byte)                         | ペイロード                  |
| SOP | type | seq | payload length | checksum | COBS encoded data ... | 0x00 |
| 1 B | 1 B  | 1 B |     2 B        |   2 B    | encoded data + delimiter  |
```

- `SOP`: パケット先頭を示す固定値 `0xAA`
- `type`: アプリケーションが定義するパケット種別
- `seq`: アプリケーションが定義するシーケンス番号
- `payload length`: COBS エンコード後のペイロード長（終端の `0x00` を含む）
- `checksum`: COBS エンコード後のペイロード全体に対する Fletcher-16
- ヘッダー内の 2 バイト値は上位バイトから格納されます．

ヘッダーは 7 byte 固定です．ペイロードの最大サイズは設定値によって決まり，デフォルトでは元データが最大 253 byte，COBS エンコード後のペイロードが最大 255 byte です．したがって，デフォルトの最大パケットサイズは 262 byte です．

## ファイル構成

```text
include/
  cobs_packet.h           パケット API
  cobs_packet_config.h    データサイズ設定
  simple_cobs.h           COBS API
src/
  cobs_packet.c           パケットの作成・解析
  simple_cobs.c           COBS エンコード・デコード
linux/
  USBSerial.hpp/.cpp      Linux シリアルポート補助クラス
stm32/
  stm32_uart_cobs_packet.h/.c
                          STM32 HAL UART 向け補助関数
```

## 使い方

パケット処理を使う場合は，`include/` をインクルードパスに追加し，`src/simple_cobs.c` と `src/cobs_packet.c` をアプリケーションと一緒にビルドしてください．UART や USB シリアルの読み書きはこのコアライブラリでは行わないため，使用する環境に応じて別途実装します．

### パケットの作成

`SendPacket_t` に送信用バッファを設定し，`cp_make_send_packet()` でパケットを作成します．作成されたパケットは `send_packet.buf` の先頭から `send_packet.packet_size` byte です．

```c
#include "cobs_packet.h"

static uint8_t tx_buf[CP_MAX_PACKET_SIZE(CP_MAX_SEND_DATA_SIZE)];
static SendPacket_t tx_packet;

int make_packet(const uint8_t *data, uint16_t data_size)
{
    if (cp_init_send_packet(&tx_packet, tx_buf, sizeof(tx_buf)) < 0) {
        return -1;
    }

    cp_status_t size = cp_make_send_packet(
        &tx_packet,
        1,          /* type */
        0,          /* seq */
        data,
        data_size
    );
    if (size < 0) {
        return (int)size;
    }

    /* transport_send(tx_packet.buf, tx_packet.packet_size); */
    return (int)tx_packet.packet_size;
}
```

`cp_make_send_packet()` の戻り値は成功時に作成したパケットサイズ，失敗時に `cp_err_t` のエラー値です．データサイズが `CP_MAX_SEND_DATA_SIZE` を超える場合や，送信バッファが足りない場合はエラーになります．

### パケットの受信・解析

受信バッファにはヘッダーとペイロードを連続して格納します．通信層でまず 7 byte のヘッダーを読み，`cp_parse_recv_header()` でペイロード長を取得してから，その長さ分のペイロードを読み込んでください．

```c
#include "cobs_packet.h"

static uint8_t rx_buf[CP_MAX_PACKET_SIZE(CP_MAX_RECV_DATA_SIZE)];
static PacketHeader_t rx_header;
static RecvPacket_t rx_packet;

int init_receiver(void)
{
    return cp_init_recv_packet(
        &rx_packet, &rx_header, rx_buf, sizeof(rx_buf)
    );
}

int parse_received_packet(void *data, uint16_t data_capacity)
{
    /* 通信層で rx_buf の先頭 7 byte を受信してから呼び出す */
    if (cp_parse_recv_header(&rx_packet) < 0) {
        return -1;
    }

    /*
     * 通信層で rx_packet.payload に
     * rx_packet.header->payload_len byte を受信する
     */
    cp_status_t packet_size = cp_parse_recv_packet(&rx_packet);
    if (packet_size < 0) {
        return (int)packet_size;
    }

    return (int)cp_get_payload_data(&rx_packet, data, data_capacity);
}
```

`cp_parse_recv_packet()` はペイロード長，終端 `0x00`，チェックサムを検査します．正常な受信データは `cp_get_payload_data()` でデコードできます．戻り値はデコード後のデータサイズで，エラー時は負の値です．

受信処理では，ヘッダーを読み込む前に `init_receiver()` を呼び出してください．`cp_parse_recv_header()` は受信ヘッダーの値を読み取ってバッファ容量を確認しますが，SOP の値によるストリーム再同期までは行いません．SOP の検出や不正データからの復帰が必要な場合は，通信層で処理してください．

### サイズ設定

デフォルトの送受信最大データ長は 253 byte です．必要に応じて [`include/cobs_packet_config.h`](include/cobs_packet_config.h) の値を変更してください．

```c
#define CP_MAX_DATA_SIZE      (253)
#define CP_MAX_SEND_DATA_SIZE (CP_MAX_DATA_SIZE)
#define CP_MAX_RECV_DATA_SIZE (CP_MAX_DATA_SIZE)
```

送信・受信の上限は個別に変更できます．バッファはそれぞれ `CP_MAX_PACKET_SIZE(CP_MAX_SEND_DATA_SIZE)`，`CP_MAX_PACKET_SIZE(CP_MAX_RECV_DATA_SIZE)` 以上のサイズで確保してください．空データのパケットは受信パーサーで拒否されるため，データ長は 1 byte 以上にしてください．

## 環境別の補助コード

### Linux

`linux/USBSerial.hpp` の `USBSerial` クラスは Linux のシリアルデバイスを開き，指定したボーレートで読み書きするための補助クラスです．デフォルトのボーレートは 115200 bps です．パケット化・解析は別途 `cobs_packet` API で行います．

```cpp
#include "USBSerial.hpp"

USBSerial serial("/dev/ttyACM0", BaudRate::b115200);
if (serial.available()) {
    // serial.send(buf, size);
    // serial.receive(buf, size);
}
```

### STM32

`stm32/stm32_uart_cobs_packet.h` は STM32 HAL の `UART_HandleTypeDef` を使う補助関数を定義しています．利用する STM32 プロジェクトの include path に `include/` と `stm32/` を追加し，対象のソースをプロジェクトに組み込んでください．STM32 HAL とプロジェクト固有の `main.h` が必要です．

UART の初期化，受信コールバック，ヘッダー受信後のペイロード受信など，実際の受信フローはアプリケーション側で構成します．

## ビルド・テスト

このリポジトリには共通のビルドシステム設定はありません．コアライブラリを使うアプリケーションのビルド設定に `src/simple_cobs.c`，`src/cobs_packet.c` を追加してください．Linux 用 `USBSerial` を使う場合は `linux/USBSerial.cpp` も追加します．

## 注意事項

- COBS はバイト列を扱います．整数や浮動小数点数を送る場合，バイト順やデータ表現はアプリケーション間で揃えてください．異なる CPU 間で構造体をそのまま送るのではなく，必要に応じて明示的にシリアライズしてください．
- `type` と `seq` の割り当て，シーケンス管理，タイムアウト，再送，SOP を使った再同期はアプリケーション側の責務です．
