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
    void setLidarValues(const std::vector<__uint16_t> &msg);
    void performAutoMeasure(std::function<void(std::vector<WORD> &)> send, std::function<void(
            void action(std::vector<WORD> &))> read);//,void (*read)(void action(std::vector<WORD>&)));
    void asyncAutoMeasure(std::function<void(std::vector<WORD> &)> send, std::vector<WORD> &msg);
    void printSegmentProfile(const std::vector <__uint16_t> &msg, std::vector<__uint16_t> &sectorNrPoints , std::vector<__uint16_t> &sectorStartPoints );
}




#endif //COMTEST_AUTOMEASURE_H
