//
// Created by eric on 30.03.22.
//


#include "AutoMeasure.h"
#include "cmath"

using namespace std;
namespace AutoMeasure {


    int pointsPerSector = -1;
    int angleStep = -1;
    int motorSpeed = -1;
    int state = -1;
    const std::vector<std::vector<WORD>> commands = {{0x0000,0x0401,0x0002},            // reset
                                                     {0x0000,0x0202,  0x0010},      //get config - global
                                                     {0x0000,0x0102},               //get status
                                                     {0x0000,0x0403, 0x0000},       //trans rotate
                                                     {0x0000,0x0404},               //trans measure
                                                     {0x0000,0x0301, 0001, 0x0100}};//get single Profile just distance



    void performAutoMeasure(std::function<void(std::vector<WORD> &)> send,
                            std::function<void(void action(std::vector<WORD> &))> read) {
        state = 0;
        while (state >= 0) {

            cout << "sending state: " << state << endl;
            if (!commands[state].empty())
                send(const_cast<vector<__uint16_t> &>(commands[state]));
            read(responseParser);
        }

    }

    void responseParser(vector<WORD> &msg) {
        cout << "parser called." << endl;
        switch (msg[0]) {
            case 0x8301:
                state = -2;
                printProfile(msg);
                break;
            case 0x8401:
                cout << "got reset" << endl;
                state=1;
                break;
            case 0x8202: //get config: global
                cout << "got config" << endl;
                if (msg[1] == 0x0010) {
                    motorSpeed = msg[3];
                    cout << "motor speed: " <<motorSpeed <<endl;
                    angleStep = msg[4];
                    cout << "angleStep: " <<angleStep <<endl;
                    pointsPerSector = 360 / (angleStep / 16);
                    cout << "pointsPerSector: " <<pointsPerSector <<endl;
                    state=2;

                } else {
                    cout << "error: unknown config" << endl;
                    state = -1;
                }
                break;
            case 0x8102: //get status
                cout << "got status" << endl;
                if (msg[2] < 4) {
                    state = msg[2] + 2;
                } else {
                    cout << "error: error state" << endl;
                    state = -1;
                }
                break;
            case 0x8403:
                cout << "got trans_rotate" << endl;
                if (msg[2] < 4) {
                    state = msg[2] + 2;
                } else {
                    cout << "error: error state" << endl;
                    state = -1;
                }
                break;
            case 0x8404:
                cout << "got trans measure" << endl;
                if (msg[2] < 4 || msg[3]) {
                    state = msg[2] + 2;
                } else {
                    cout << "error: error state" << endl;
                    state = -1;
                }
                break;
            case 0xFF00:
                cout << "error: ServiceFailure" << endl;
                state = -1;
                break;
            default:
                cout << "error: Unknown response: " << msg[0]<< endl;
                state = -1;
        }
    }

    void printProfile(const vector<__uint16_t> &msg) { //TODO add multi sector support
        WORD profileFormat = msg[1];
        char numberOfSectors = (char) (msg[2] & 0xFF);
        char numberOfLayers = (char) (msg[2] >> 8);
        int msgData = 3;
        unsigned long offset;
        bool distance = profileFormat & 0b100000000, direction = profileFormat & 0b1000000000, echo =
                profileFormat & 0b10000000000;


        if (distance || direction || echo) { // bit 8 9 10
            offset = bitset<16>(profileFormat & 0xFF).count();
            if (pointsPerSector != -1) {
                printf("\tpoints: ");
                for (int iPoints = 0; iPoints < pointsPerSector; ++iPoints) {
                    unsigned long iterationOffset = offset + (distance + direction + echo) * iPoints;
                    //printf("(");
                    if (distance) {
                        //printf("%.4f", (float) msg[msgData + iterationOffset] / 256);
                        printf("%.4f\t%.4f",((float) msg[msgData + iterationOffset] / 256)*sin((angleStep/16)*iPoints*M_PI/180),((float) msg[msgData + iterationOffset] / 256)*cos((angleStep/16)*iPoints*M_PI/180));
                    }
                    if (direction) {
                        printf(",%.4f", (float) msg[msgData + iterationOffset + distance] / 16);
                    }
                    if (echo) {
                        printf(",%i", msg[msgData + iterationOffset + distance + direction]);
                    }
                    printf("\n");
                    //printf("\n)");
                }
                printf("\n");
            }
        }
    }
}