//
// Created by eric on 30.03.22.
//

#ifndef COMTEST_AUTOMEASURE_H
#define COMTEST_AUTOMEASURE_H
#include "common .h"
#include "vector"
#include <cstdlib>
#include "iostream"
#include "bitset"
#include "functional"
namespace AutoMeasure {
    void responseParser(std::vector<WORD> &msg);

    void printProfile(const std::vector<__uint16_t> &msg);

    void performAutoMeasure(std::function<void(std::vector<WORD> &)> send, std::function<void(
            void action(std::vector<WORD> &))> read);//,void (*read)(void action(std::vector<WORD>&)));
}




#endif //COMTEST_AUTOMEASURE_H
