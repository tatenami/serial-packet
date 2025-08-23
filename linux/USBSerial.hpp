#ifndef USB_SERIAL_H
#define USB_SERIAL_H

#include <cstdio>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/serial.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdint.h>

enum class ReadMode {
  Blocking, NonBlocking
};

// from termios-baud.h
enum class BaudRate {
  b50	 = B50,
  b75	 = B75,
  b110 = B110,
  b134 = B134,
  b150 = B150,
  b200 = B200,
  b300 = B300,
  b600 = B600,
  b1200	= B1200,
  b1800	= B1800,
  b2400	= B2400,
  b4800	= B4800,
  b9600	= B9600,
  b19200 = B19200,
  b38400 = B38400,
  b57600 = B57600,
  b115200 = B115200,
  b230400 = B230400,
};

class USBSerial {
 private:
  int fd_;
  bool available_;
  speed_t baud_rate_;
  termios tio_config_;
  std::string dev_file_;

 private:
  void setLowLatency();
  void initTermios(BaudRate baudrate);
  int  openFile(ReadMode mode);

  // デバイスの切断チェック
  inline bool isDeviceConnect() {
    if (errno == ENXIO || errno == EIO || errno == ENODEV) {
      available_ = false;
      return false;
    };

    return true;
  }

 public:
  USBSerial(const char* dev_file, 
            BaudRate baudrate = BaudRate::b115200, 
            ReadMode mode = ReadMode::Blocking);
  ~USBSerial();

  bool available();
  int send(void *buf, size_t size);
  int receive(void *buf, size_t size);

  template <typename T>
  int send(T *buf) {
    int size = sizeof(T);
    int wsize;
    uint8_t *buf_p = (uint8_t *)buf;

    for (int i = 0; i < size; i++) {
      wsize = write(fd_, &buf_p[i], 1);

      if (!isDeviceConnect()) return 0;
    }
    
    return size;
  }

  template <typename T>
  int receive(T *buf) {
    int size = sizeof(T);
    int rsize;
    uint8_t *buf_p = (uint8_t *)buf;

    for (int i = 0; i < size; i++) {
      rsize = read(fd_, &buf_p[i], 1);

      if (!isDeviceConnect()) return 0;
    }
    
    return size;
  }
};

#endif // USB_SERIAL_H