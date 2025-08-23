#include "USBSerial.hpp"

USBSerial::USBSerial(const char *dev_file, BaudRate baudrate, ReadMode mode) {
  dev_file_ = dev_file;

  fd_ = openFile(mode);
  available_ = (fd_ < 0) ? false : true;

  initTermios(baudrate);
}

USBSerial::~USBSerial() {
  close(fd_);
}

void USBSerial::initTermios(BaudRate baudrate) {
  baud_rate_ = (speed_t)baudrate;

  // 生データを扱う設定
  cfmakeraw(&tio_config_); 

  // 受信有効化 モデム制御無効化 データ長8Bit
  tio_config_.c_cflag &= ~CSIZE;
  tio_config_.c_cflag |= (CREAD | CLOCAL | CS8);  

  // 1 Byte 受信するまで read 内でブロッキング(処理待機)
  tio_config_.c_cc[VMIN] = 1;
  tio_config_.c_cc[VTIME] = 0;

  cfsetspeed(&tio_config_, (speed_t)baudrate); // 送受信速度設定
  setLowLatency();

  // 設定の反映
  if (tcsetattr(fd_, TCSANOW, &tio_config_)) {
    available_ = false;
  }
}

void USBSerial::setLowLatency() {
  // カーネルのデータ受信時の待ち時間を無効化
  serial_struct serial_setting;
  if (ioctl(fd_, TIOCGSERIAL, &serial_setting) < 0) {
    available_ = false;
    return;
  }

  serial_setting.flags |= ASYNC_LOW_LATENCY;

  if (ioctl(fd_, TIOCSSERIAL, &serial_setting) < 0) {
    available_ = false;
  }
}

int USBSerial::openFile(ReadMode mode) {
  int fd;
  int flag;
  
  flag = (mode == ReadMode::Blocking) ? 0 : O_NONBLOCK;
  flag |= O_RDWR;
  
  fd = open(dev_file_.c_str(), flag);
  return fd;
}

bool USBSerial::available() {
  return available_;
}

int USBSerial::send(void *buf, size_t size) {
  int wsize;
  uint8_t *buf_p = (uint8_t *)buf;

  for (int i = 0; i < size; i++) {
    wsize = write(fd_, &buf_p[i], 1);

    if (!isDeviceConnect()) return 0;
  }
  
  return size;
}


int USBSerial::receive(void *buf, size_t size) {
  int rsize;
  uint8_t *buf_p = (uint8_t *)buf;

  for (int i = 0; i < size; i++) {
    rsize = read(fd_, &buf_p[i], 1);

    if (!isDeviceConnect()) return 0;
  }
  
  return size;
}

