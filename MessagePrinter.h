//
// Created by eric on 30.03.22.
//

#ifndef COMTEST_MESSAGEPRINTER_H
#define COMTEST_MESSAGEPRINTER_H

#define printBuffer_size 8192
#include "common .h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include "regex"
#include "iostream"
#include "bitset"

void printResponse(std::vector <WORD> &msg);
void printPortVal(WORD portVal, char* indent, char* printBuffer);
void printSenStat(WORD senStat, char* indent, char* printBuffer);
void printProfile(const std::vector<__uint16_t> &msg, char* printBuffer);
#endif //COMTEST_MESSAGEPRINTER_H
