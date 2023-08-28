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
    //int pointsPerSector = -1;
    int angleStep = -1;
    int motorSpeed = -1;
    int state = 0;
    int minX,maxX,minY,maxY;
    std::vector<WORD> pointsPerSector;
    std::vector<WORD> sectorStartAngles;

    const std::vector<std::vector<WORD>> commands = {//{0x0401,0x0002},            // reset
                                                     //{0x0202,  0x0010},      //get config - global
                                                     {0x0102},               //get status
                                                     {0x0403, 0x0000},       //trans rotate
                                                     {0x0404},               //trans measure
                                                     {0x0301, 0x0001, 0x00B0},
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
                if ((msg[1]&0b10110000)==176 && (angleStep<0 || pointsPerSector.empty() || sectorStartAngles.empty())) {
                    setLidarValues(msg);
                    state = 4;
                }else {
                    state = 5;
                    //printProfile(msg);
                    printSegmentProfile(msg,pointsPerSector,sectorStartAngles);

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
                    //pointsPerSector = 360 / (angleStep / 16);
                    //std::cout << "pointsPerSector: " << pointsPerSector <<std::endl;
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


    void printSegmentProfile(const std::vector <__uint16_t> &msg, std::vector<__uint16_t> &sectorNrPoints , std::vector<__uint16_t> &sectorStartPoints ){

        WORD profileFormat = msg[1];
        bool distance = profileFormat & 0b100000000, direction = profileFormat & 0b1000000000, echo =
                profileFormat & 0b10000000000;
        int dataOffset=3; //TODO calc for Frame

        pos.clear();

        unsigned long offset= std::bitset<16>(profileFormat & 0xFF).count()+dataOffset;
        for(int iSegments = 0 ; iSegments < sectorNrPoints.size(); ++iSegments){
            //do someLoop
            //printf("sectorNR: %i, pointsPerSector: %i:",iSegments,pointsPerSector[iSegments]);
            double angleOffset= ((double)sectorStartPoints[iSegments])/16;
            printf("SectorStart: %f",angleOffset);
            for(int i=0; i < sectorNrPoints[iSegments]; ++i){
                int iterationOffset = i* distance + direction + echo;

                if (distance){
                    double x = ((double) msg[offset + iterationOffset] / 256)*sin((((double)angleStep/16)*i+((double)sectorStartPoints[iSegments]/16))*M_PI/180);
                    double y = ((double) msg[offset + iterationOffset] / 256)*cos((((double)angleStep/16)*i+((double)sectorStartPoints[iSegments]/16))*M_PI/180);
                    //printf("(%.2f,%.2f),",x,y);
                    //printf("%i,",msg[offset + iterationOffset]);
                    minX= std::min((int)std::floor(x),minX);
                    minY= std::min((int)std::floor(y),minY);
                    maxX= std::max((int)std::ceil(x),maxX);
                    maxY= std::max((int)std::ceil(y),maxY);
                    pos.push_back(boost::make_tuple(x,y));

                }

            }
            offset+= std::bitset<16>(profileFormat & 0xF8).count() + sectorNrPoints[iSegments] * (distance + direction + echo); //+nPoints*data 1111 1000
            printf("\n");
        }
        gp << "set xrange [" << minX << ":" << minX+std::max(maxX-minX,maxY-minY) << "]\nset yrange [" << minY<< ":" << minY+std::max(maxX-minX,maxY-minY) << "]\nset size ratio -1\n";
        gp << "plot '-' title 'Lidar'\n";
        gp.send1d(pos);
    }



    void printProfile(const std::vector<__uint16_t> &msg) { //TODO add multi sector support
        int defaultstartSector=0;
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
            if (!pointsPerSector.empty()) {
                printf("\tpoints:\t\t");
                for (int iPoints = 0; iPoints < pointsPerSector[defaultstartSector]; ++iPoints) {
                    unsigned long iterationOffset = offset + (distance + direction + echo) * iPoints;
                    //printf("(");
                    if (distance) {
                        double x = ((double) msg[msgData + iterationOffset] / 256)*sin(((double)angleStep/16)*iPoints*M_PI/180);
                        double y = ((double) msg[msgData + iterationOffset] / 256)*cos(((double)angleStep/16)*iPoints*M_PI/180);
                        //printf("%i,",msg[msgData + iterationOffset]);
                        //printf("(%.2f,%.2f),",x,y);
                        /*minX= std::min((int)std::floor(x),minX);
                        minY= std::min((int)std::floor(y),minY);
                        maxX= std::max((int)std::ceil(x),maxX);
                        maxY= std::max((int)std::ceil(y),maxY);*/
                        //printf("%.4f", (float) msg[msgData + iterationOffset] / 256);
                        //printf("%.4f\t%.4f",((float) msg[msgData + iterationOffset] / 256)*sin((angleStep/16)*iPoints*M_PI/180),((float) msg[msgData + iterationOffset] / 256)*cos((angleStep/16)*iPoints*M_PI/180));
                        //pos.push_back(boost::make_tuple(x,y));
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
                printf("\ntocheck:\t\t");
                std::vector<__uint16_t> testSector={0,44,0,180};
                std::vector<__uint16_t> testStartPoints ={0,125,147,270};
                printSegmentProfile(msg,pointsPerSector,sectorStartAngles);
            }

        }
        //gp << "set xrange [-1:4]\nset yrange [-3:1]\nset size ratio -1\n";

        /*gp << "set xrange [" << minX << ":" << minX+std::max(maxX-minX,maxY-minY) << "]\nset yrange [" << minY<< ":" << minY+std::max(maxX-minX,maxY-minY) << "]\nset size ratio -1\n";
        gp << "plot '-' title 'Lidar'\n";
        gp.send1d(pos);//*/

    }
    void setLidarValues(const std::vector<__uint16_t> &msg){ //TODO DELETE usless stuff and implement new
        printf("getting lidarValues\n");
        WORD profileFormat = msg[1];
        pointsPerSector.clear();
        sectorStartAngles.clear();
        bool distance = profileFormat & 0b100000000, direction = profileFormat & 0b1000000000, echo =
                profileFormat & 0b10000000000;
        int dataOffset=3; //TODO calc for Frame
        char numberOfSectors= (char)(msg[2] & 0xFF);
        unsigned long offset= std::bitset<16>(profileFormat & 0x0F).count()+dataOffset;
        for(int iSegments = 0 ; iSegments < numberOfSectors; ++iSegments){


            angleStep= msg[offset];
            WORD tmpPointsPerSector= msg[offset+1];
            WORD sectorStartDir= msg[offset+2+((profileFormat&0b1000000)>0)];
            printf("SectroDir: %i ",sectorStartDir);
            printf("pointsPerSec: %i ",tmpPointsPerSector);
            pointsPerSector.push_back(tmpPointsPerSector);
            sectorStartAngles.push_back(sectorStartDir);
            offset+= std::bitset<16>(profileFormat & 0xF8).count() + tmpPointsPerSector * (distance + direction + echo)+((profileFormat&0b1000000000000)>0); //TODO add TEND to offset
            printf("\n");
        }
        printf("angleStep: %i\n",angleStep);
    }

    /*void setLidarValuesOld(const std::vector<__uint16_t> &msg) { //TODO add multi sector support
        WORD profileFormat=msg[1];
        char numberOfSectors= (char)(msg[2] & 0xFF);
        char numberOfLayers= (char)(msg[2] >> 8);
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
        /*if (profileFormat&0b10000){ // bit 4
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
    //}


}


