//
// Created by eric on 01.04.22.
//

#ifndef COMTEST_MESSAGEHANDLERENTERNET_H
#define COMTEST_MESSAGEHANDLERENTERNET_H
#include "common .h"
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <array>
#include "functional"
std::vector<__uint8_t> buildTelegram(std::vector<WORD> &msg);
bool readBuffer(const char *readBuf, std::vector<char> &msgBuf, int bytesRead, std::function<void(std::vector<__uint16_t>&)>&action);
#endif //COMTEST_MESSAGEHANDLERENTERNET_H
