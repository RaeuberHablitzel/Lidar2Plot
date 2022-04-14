#define buf_size 256
//#define baud_rate B115200
//#define com_port "/dev/ttyACM0"
//#define com_port "/dev/ttyUSB0"

//#define printColorRed "\033[0;31m"
//#define printColorNormal "\033[0m"

//#define WORD __uint16_t
//#define BYTE __uint8_t

// C library headers
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include "regex"
#include "iostream"
#include "bitset"


//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <cerrno> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include <sys/ioctl.h>


#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

