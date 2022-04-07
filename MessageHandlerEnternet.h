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
namespace msgHE{
    std::vector<__uint8_t> buildTelegram(std::vector<WORD> &msg);
    bool readBuffer(const unsigned char *readBuf, std::vector<unsigned char> &msgBuf, int bytesRead, std::function<void(std::vector<__uint16_t>&)>&action);
    int convertMsgToWords(std::vector<char> &msg,  std::vector<__uint16_t> &res);
}
#endif //COMTEST_MESSAGEHANDLERENTERNET_H
