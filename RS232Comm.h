//
// Created by eric on 03.04.22.
//

#ifndef COMTEST_RS232COMM_H
#define COMTEST_RS232COMM_H

#define buf_size 256
#define baud_rate B115200
#define com_port "/dev/ttyUSB0"

#include "common .h"
#include "cstdlib"
#include "vector"
#include "iostream"
#include "MessageHandler.h"
#include "MessagePrinter.h"
#include "AutoMeasure.h"

//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <cerrno> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include <sys/ioctl.h>

int r232Main();

#endif //COMTEST_RS232COMM_H
