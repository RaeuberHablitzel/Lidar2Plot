//
// Created by eric on 30.03.22.
//

#include <thread>
#include <chrono>
#include "AutoMeasure.h"
#include <cmath>
#include <array>
#include <boost/tuple/tuple.hpp>
#include "gnuplot-iostream.h"

namespace AutoMeasure {


    Gnuplot gp;
    std::vector<boost::tuple<double, double> > pos;
    int pointsPerSector = -1;
    int angleStep = -1;
    int motorSpeed = -1;
    int state = 0;
    int minX,maxX,minY,maxY;
    const std::vector<std::vector<WORD>> commands = {//{0x0401,0x0002},            // reset
                                                     //{0x0202,  0x0010},      //get config - global
                                                     {0x0102},               //get status
                                                     {0x0403, 0x0000},       //trans rotate
                                                     {0x0404},               //trans measure
                                                     {0x0301, 0x0001, 0x0030},
                                                     {0x0301, 0x0000, 0x0100},
                                                     {}};//get single Profile just distance
    const std::vector<std::string> states={//"reset",
                                      //"get config",
                                      "get state",
                                      "trans rotate",
                                      "trans measure",
                                      "get settings",
                                      "get signal",
                                      "wait"};


    void asyncAutoMeasure(std::function<void(std::vector<WORD> &)> send, std::vector<WORD> &msg){
        switch (msg[0]) {
            case 0x8301:
                if ((msg[1]&0b110000)==48) {
                    setLidarValues(msg);
                    state = 4;
                }else {
                    state = 5;
                    printProfile(msg);
                }
                break;
            case 0x8102: //get status
                std::cout << "got status" << std::endl;
                if (msg[2] < 4) {
                    state = msg[2] ;//+ 2;

                } else {
                    std::cout << "error: error state" << std::endl;
                    state = -1;
                }
                break;
            case 0x8403:
                std::cout << "got trans_rotate" << std::endl;
                if (msg[2] < 4) {
                    state = msg[2];// + 2;
                } else {
                    std::cout << "error: error state" << std::endl;
                    state = -1;
                }
                break;
            case 0x8404:
                std::cout << "got trans measure" << std::endl;
                if (msg[2] < 4 || msg[3]) {
                    state = msg[2];// + 2;
                } else {
                    std::cout << "error: error state" << std::endl;
                    state = -1;
                }
                break;
            case 0xFF00:
                std::cout << "error: ServiceFailure" << std::endl;
                state = -1;
                break;
            default:
                std::cout << "error: Unknown response: " << msg[0]<< std::endl;
                state = -1;
        }
        if (state>=0) {
            std::cout << "sending state: " << states[state] << std::endl;
            if (!commands[state].empty())
                send(const_cast<std::vector<__uint16_t> &>(commands[state]));
        }
    }

    void performAutoMeasure(std::function<void(std::vector<WORD> &)> send,
                            std::function<void(void action(std::vector<WORD> &))> read) {


        //gp << "set xrange [-1:4]\nset yrange [-3:1]\nset size ratio -1\n";

        state = 0;
        while (state >= 0) {

            std::cout << "sending state: " << states[state] << std::endl;
            if (!commands[state].empty())
                send(const_cast<std::vector<__uint16_t> &>(commands[state]));
            read(responseParser);
        }

    }

    void responseParser(std::vector<WORD> &msg) {
        std::cout << "parser called." << std::endl;
        switch (msg[0]) {
            case 0x8301:
                if ((msg[1]&0b110000)==48) {
                    setLidarValues(msg);
                    state = 4;
                }else {
                    state = 5;
                    printProfile(msg);
                }
                break;
            case 0x8401:
                std::cout << "got reset" << std::endl;
                state=1;
                break;
            case 0x8202: //get config: global
                std::cout << "got config" << std::endl;
                if (msg[1] == 0x0010) {
                    motorSpeed = msg[3];
                    std::cout << "motor speed: " <<motorSpeed <<std::endl;
                    angleStep = msg[4];
                    std::cout << "angleStep: " <<angleStep <<std::endl;
                    pointsPerSector = 360 / (angleStep / 16);
                    std::cout << "pointsPerSector: " <<pointsPerSector <<std::endl;
                    state=2;

                } else {
                    std::cout << "error: unknown config" << std::endl;
                    state = -1;
                }
                break;
            case 0x8102: //get status
                std::cout << "got status" << std::endl;
                if (msg[2] < 4) {
                    state = msg[2] ;//+ 2;

                } else {
                    std::cout << "error: error state" << std::endl;
                    state = -1;
                }
                break;
            case 0x8403:
                std::cout << "got trans_rotate" << std::endl;
                if (msg[2] < 4) {
                    state = msg[2];// + 2;
                } else {
                    std::cout << "error: error state" << std::endl;
                    state = -1;
                }
                break;
            case 0x8404:
                std::cout << "got trans measure" << std::endl;
                if (msg[2] < 4 || msg[3]) {
                    state = msg[2];// + 2;
                } else {
                    std::cout << "error: error state" << std::endl;
                    state = -1;
                }
                break;
            case 0xFF00:
                std::cout << "error: ServiceFailure" << std::endl;
                state = -1;
                break;
            default:
                std::cout << "error: Unknown response: " << msg[0]<< std::endl;
                state = -1;
        }
    }

    void printProfile(const std::vector<__uint16_t> &msg) { //TODO add multi sector support
        WORD profileFormat = msg[1];
        char numberOfSectors = (char) (msg[2] & 0xFF);
        char numberOfLayers = (char) (msg[2] >> 8);
        int msgData = 3;
        unsigned long offset;
        bool distance = profileFormat & 0b100000000, direction = profileFormat & 0b1000000000, echo =
                profileFormat & 0b10000000000;

        pos.clear();
        if (distance || direction || echo) { // bit 8 9 10
            offset = std::bitset<16>(profileFormat & 0xFF).count();
            if (pointsPerSector != -1) {
                printf("\tpoints: ");
                for (int iPoints = 0; iPoints < pointsPerSector; ++iPoints) {
                    unsigned long iterationOffset = offset + (distance + direction + echo) * iPoints;
                    //printf("(");
                    if (distance) {
                        double x = ((double) msg[msgData + iterationOffset] / 256)*sin(((double)angleStep/16)*iPoints*M_PI/180);
                        double y = ((double) msg[msgData + iterationOffset] / 256)*cos(((double)angleStep/16)*iPoints*M_PI/180);
                        printf("(%.2d,%.2d),",x,y);
                        minX= std::min((int)std::floor(x),minX);
                        minY= std::min((int)std::floor(y),minY);
                        maxX= std::max((int)std::ceil(x),maxX);
                        maxY= std::max((int)std::ceil(y),maxY);
                        //printf("%.4f", (float) msg[msgData + iterationOffset] / 256);
                        //printf("%.4f\t%.4f",((float) msg[msgData + iterationOffset] / 256)*sin((angleStep/16)*iPoints*M_PI/180),((float) msg[msgData + iterationOffset] / 256)*cos((angleStep/16)*iPoints*M_PI/180));
                        pos.push_back(boost::make_tuple(x,y));
                    }
                    if (direction) {
                        printf(",%.4f", (float) msg[msgData + iterationOffset + distance] / 16);
                    }
                    if (echo) {
                        printf(",%i", msg[msgData + iterationOffset + distance + direction]);
                    }
                    //printf("\n");
                    //printf("\n)");
                }
                printf("\n");
            }
        }
        //gp << "set xrange [-1:4]\nset yrange [-3:1]\nset size ratio -1\n";

        gp << "set xrange [" << minX << ":" << minX+std::max(maxX-minX,maxY-minY) << "]\nset yrange [" << minY<< ":" << minY+std::max(maxX-minX,maxY-minY) << "]\nset size ratio -1\n";
        gp << "plot '-' title 'Lidar'\n";
        gp.send1d(pos);

    }
    void setLidarValues(const std::vector<__uint16_t> &msg) { //TODO add multi sector support
        WORD profileFormat=msg[1];
        //char numberOfSectors= (char)(msg[2] & 0xFF);
        //char numberOfLayers= (char)(msg[2] >> 8);
        int msgData= 3;
        unsigned long offset;
        //bool distance=profileFormat&0b100000000, direction=profileFormat&0b1000000000, echo=profileFormat&0b10000000000;
        /*
        snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tProfileFormat: %04X\n",profileFormat);
        snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tNumber of Layers: %i\n",numberOfLayers);
        snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tNumber of Sectors: %i\n",numberOfSectors);
        snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tdist: %i,dir: %i,echo: %i\n",distance,direction,echo);
        *
        if (profileFormat&0b1){ // bit 0
            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tProfile sent: %i \n",msg[msgData]);
        }
        if (profileFormat&0b10){ // bit 1
            offset = bitset<16>(profileFormat&1).count();
            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tProfile count: %i\n",msg[msgData+(offset)]);
        }
        if (profileFormat&0b100){ // bit 2
            offset = bitset<16>(profileFormat&0b11).count();
            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tcurrentLayer: %i\n",msg[msgData+(offset)]);
        }
        if (profileFormat&0b1000){ //bit 3
            offset = bitset<16>(profileFormat&0b111).count();
            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tSector number: %i\n",msg[msgData+(offset)]);
        }*/
        if (profileFormat&0b10000){ // bit 4
            offset = std::bitset<16>(profileFormat&0b1111).count();
            angleStep=msg[msgData+(offset)];
            printf("angleStep: %i\n",angleStep);
        }
        if (profileFormat&0b100000){ // bit 5
            offset = std::bitset<16>(profileFormat&0b11111).count();
            pointsPerSector=msg[msgData+(offset)];
            printf("pointsPerSector: %i\n",pointsPerSector);


        }/*
        if (profileFormat&0b1000000){ // bit 6
            offset = bitset<16>(profileFormat&0b111111).count();
            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tTimestamp start Sector: %i\n",msg[msgData+(offset)]);
        }
        if (profileFormat&0b10000000){ // bit 7
            offset = bitset<16>(profileFormat&0b1111111).count();
            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tstart Angle: %f°\n",(float)msg[msgData+(offset)]/16);
        }
        if (distance||direction||echo){ // bit 8 9 10
            offset = bitset<16>(profileFormat&0xFF).count();
            if (pointsPerSector!=-1){
                snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tpoints: ");
                for (int iPoints = 0; iPoints < pointsPerSector; ++iPoints) {
                    unsigned long iterationOffset = offset+(distance+direction+echo)*iPoints;
                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"(");
                    if (distance) {
                        snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"%.4f",(float)msg[msgData+iterationOffset]/256);
                    }
                    if (direction) {
                        snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),",%.4f",(float)msg[msgData+iterationOffset+distance]/16);
                    }
                    if (echo) {
                        snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),",%i",msg[msgData+iterationOffset+distance+direction]);
                    }

                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\t)");
                }
                snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\n");
            }
        }*/
        // TEND, ENDDRIR, SENSTAT mayby von len aus
    }


}


