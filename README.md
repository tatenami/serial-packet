# serial-packet
マイコン向けシリアル通信用パケットの C/C++ ライブラリ

## 概要
 - 最大 253 Byte の任意のデータをパケットにする
 - データは COBS エンコードを施し，パケットのペイロードとする

### パケット構造
`| HEADER (3 Byte) | PAYLOAD (max 255 Byte) |`
 - `HEADER`
   - `| SOP (start of packet) | paylaod size | checksum of payload |`
 - `PAYLOAD`
   - `| COBS encoded data |`

### 各種ライブラリ
 - `cobs_packet_config.h`・`cobs_packet.h/.c`
   - パケットのメイン処理群
 - `COBS/`
   - COBS エンコード・デコード処理
 - `linux/`
   - Linux 用簡易シリアル通信ライブラリ
 - `stm32/`
   - STM32 マイコン用簡易 UART 通信ライブラリ
 - `sample/`
   - Linux・STM32 の COBS パケットを用いた通信のサンプルコード

## Usage
パケット関連の処理群は `cobs_packet.h/.c` に記述．
`COBS/SimpleCOBS.h/c` 及び `cobs_packet_config.h` に依存するため，使用時はこれらもセットで使う．

### パケット用バッファの設定
`cobs_packet.h/.c` は，送信・受信用パケットのバッファをライブラリ内部で定義・利用しており，
デフォルトでは `SimpleCOBS.h` で設定している最大データ長の 253 Byte に合わせて `cobs_packet_config.h` でバッファのサイズを定義している． 

このバッファサイズの最大サイズの変更する場合は，`cobs_packet_config.h` 中でマクロで定義されている
送信・受信それぞれのサイズを書き換える．

```c
// デフォルト: 253 byte (パケットで扱える最大データ長)
#define MAX_DATA_SIZE_DEFAULT COBS_MAX_DATA_SIZE

#define MAX_SEND_DATA_SIZE MAX_DATA_SIZE_DEFAULT
#define MAX_RECV_DATA_SIZE MAX_DATA_SIZE_DEFAULT

// 変更例
#define MAX_SEND_DATA_SIZE 4
#define MAX_RECV_DATA_SIZE 8
```

### Linux
ファイル構成例
 - USBSerial.hpp/cpp は Linux で簡易的な USB シリアルを利用するためのライブラリ
```
.
├── SimpleCOBS.c
├── SimpleCOBS.h
├── cobs_packet_config.h
├── cobs_packet.h
├── cobs_packet.c
├── USBSerial.hpp
├── USBSerial.cpp
└── main.cpp
```
最低限必要なファイルは以下の 5つ
 - `SimpleCOBS.h`
 - `SimpleCOBS.c`
 - `cobs_packet_config.h`
 - `cobs_pakcet.h`
 - `cobs_packet.c`

### stm32
ファイル構成例
```
Core/
  ├── Inc/
  │   ├── SimpleCOBS.h
  │   ├── cobs_packet_config.h
  │   ├── cobs_packet.h
  │   ├── stm32_uart_cobs_packet.h
  │   └── ... (other headers)
  └── Src/
      ├── SimpleCOBS.c
      ├── cobs_packet.c
      ├── stm32_uart_cobs_packet.c
      └── ... (other sources)
```

## サンプル
`sample/` に Linux と STM32 マイコンボード (Nucleo) 間で
簡易的な COBS パケットを用いたシリアル通信の例を用意しています．

Linux 側から角度 `theta` の値を送り，STM 側が `sin(theta)` の値を計算して返すというものです．　
```
    --> pakcet (data: theta)
| Linux | <--(USB)--> | STM32 (Nucleo) |
    packet (data: sin(theta)) <--
```

### 実験環境 (作成者)
 - Linux PC: Arch Linux
 - STM32: Nucleo F767ZI

### 注意
STM32 マイコンは，ボード上に ST-Link デバッガーが付属しており，それとメイン MCU が UART 接続可能な Nucleo-64 or Nucleo-144 を利用してください．
CubeMX で Nucleo ボードのプロジェクトを作成した際に最初から有効化されている UART が USB を介して PC と通信可能なものなので，それを使用すれば問題ありません．