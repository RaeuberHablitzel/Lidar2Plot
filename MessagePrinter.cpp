//
// Created by eric on 30.03.22.
//

#include "MessagePrinter.h"


using namespace std;

void printResponse(vector<WORD> &msg){
    printf("header: %04X\n",msg[0]);
    char singleIndent[]= "\t";
    // translate header
    //  high-byte as Service group
    //  loy-byte as Service
    switch ((msg[0] & 0xff00) >> 8)
    {
        case 0x81:
            printf("Response to Status Service: ");
            switch (msg[0] & 0xff)
            {
                case 0x01:
                    printf("GET_IDENTIFICATION \n\t");
                    for (int i= 1; i < msg[1] - 2; ++i){
                        printf("%c%c",(msg[i]>>8),msg[i]&0xFF);
                    }
                    printf("\n");
                    printSenStat(msg[8], singleIndent);

                    break;
                case 0x02:
                    printf("GET_STATUS\n");
                    printSenStat(msg[2], singleIndent);
                    break;
                case 0x04:
                    printf("GET_SIGNAL\n");
                    printPortVal(msg[1], singleIndent);
                    break;
                case 0x05:
                    printf("SET_SIGNAL\n");
                    if (msg[1] == 0xFFFF){
                        printf("\tfailed to set signals");
                    }else{
                        printPortVal(msg[1], singleIndent);
                    }
                    break;
                case 0x6:
                    printf("REGISTER_APPLICATION\n");
                    printf("\tRegistration ");
                    if(!msg[1]){
                        printf("successful\n");
                    }else{
                        printf("failed\n");
                    }
                    break;

                default:
                    printf("\n");
                    break;
            }
            break;
        case 0x82:
            printf("Response to Configuration Services: ");
            switch (msg[0] & 0xff)
            {
                case 0x01:
                    printf("SET_CONFIG\n"); // TODO Response Configuration Services: SET_CONFIG

                    switch (msg[1]) {
                        case 0x0000:
                            printf("request successful\n");
                            break;
                        case 0xFFFF:
                            printf("request not successful\n");
                            break;
                        default:
                            printf("invalid\n");

                    }
                    break;
                case 0x02:
                    printf("GET_CONFIG\n");
                    switch (msg[1])
                    {
                        case 0x0001: // RS232/422
                            printf("\tRS232 / RS422 Config:\n\t\tBaud rate: ");
                            switch (msg[2]) // baud rate
                            {
                                case 0x0001:
                                    printf("4800 bd\n");
                                    break;
                                case 0x0002:
                                    printf("9600 bd\n");
                                    break;
                                case 0x0003:
                                    printf("19200 bd\n");
                                    break;
                                case 0x0004:
                                    printf("38400 bd\n");
                                    break;
                                case 0x0005:
                                    printf("57600 bd\n");
                                    break;
                                case 0x0006:
                                    printf("115200 bd\n");
                                    break;

                                default:
                                    printf("unknown/invalid\n");
                                    break;
                            }

                            printf("\t\tParity: ");
                            switch (msg[3]) // parity
                            {
                                case 0x0000:
                                    printf("No parity\n");
                                    break;
                                case 0x0001:
                                    printf("Even parity\n");
                                    break;
                                case 0x0002:
                                    printf("Odd parity\n");
                                    break;

                                default:
                                    printf("unknown/invalid\n");
                                    break;
                            }

                            printf("\t\tNumber of Stop bits: ");
                            switch (msg[4]) // stop bits
                            {
                                case 0x0001:
                                    printf("1\n");
                                    break;
                                case 0x0002:
                                    printf("2\n");
                                    break;

                                default:
                                    printf("unknown/invalid\n");
                                    break;
                            }
                            printf("\t\tBits per character: ");
                            if (msg[5] <= 8)
                            {
                                printf("%i\n", msg[5]);
                            }
                            else
                            {
                                printf("invalid\n");
                            }
                            break;
                        case 0x0002:
                            printf("\tCAN Config:\n\t\tdata transmission rate: ");
                            switch (msg[2]) // transmission rate
                            {
                                case 0x0000:
                                    printf("10 kBit/s\n");
                                    break;
                                case 0x0001:
                                    printf("20 kBit/s\n");
                                    break;
                                case 0x0002:
                                    printf("50 kBit/s\n");
                                    break;
                                case 0x0003:
                                    printf("125 kBit/s\n");
                                    break;
                                case 0x0004:
                                    printf("250 kBit/s\n");
                                    break;
                                case 0x0005:
                                    printf("500 kBit/s\n");
                                    break;
                                case 0x0006:
                                    printf("1 MBit/s\n");
                                    break;

                                default:
                                    printf("unknown/invalid\n");
                                    break;
                            }
                            printf("\t\tBasic  value of the host CAN identifier: %04x\n", msg[3]);
                            printf("\t\tMask value of the host CAN identifier: %04x\n", msg[4]);
                            printf("\t\tBasic value of the LD-OEM/LD-LRS CAN identifier: %04x\n", msg[5]);
                            printf("\t\tBroadcast-ID (0xFFFF if disabled): %04x\n", msg[6]);
                            break;
                        case 0x0005:
                            printf("\tEthernet Config:\n");
                            printf("\t\tIP Address:\t%i.%i.%i.%i\n", msg[2], msg[3], msg[4], msg[5]);
                            printf("\t\tSubnet mask:\t%i.%i.%i.%i\n", msg[6], msg[7], msg[8], msg[9]);
                            printf("\t\tStandard gateway:\t%i.%i.%i.%i\n", msg[10], msg[11], msg[12], msg[13]);
                            printf("\t\tNode ID: %04x\n",msg[14]);
                            printf("\t\tTCP/IP Transparent port: %04x\n",msg[15]);

                            break;
                        case 0x0010: // Global config
                            printf("\tGlobal Config:\n\t\tSensorID: ");
                            if (msg[2] <= 254){
                                printf("%02x\n",msg[2]);
                            }else{
                                printf("invalid\n");
                            }
                            printf("\t\tNominal motor speed: ");
                            if (msg[3] <= 20 && msg[3] >= 5){
                                printf("%iHz\n",msg[3]);
                            }else{
                                printf("invalid\n");
                            }
                            printf("\t\tAngle step: ");
                            if (msg[4] >= 1 ){ //TODO divisor
                                float f= ((float)msg[4]) / 16;
                                printf("%f° raw: %i\n",f,msg[4]);
                            }else{
                                printf("invalid\n");
                            }


                            break;

                        default:
                            break;
                    }
                    break;
                case 0x03:
                    printf("SET_SYNC_ABS\n"); // TODO Response Configuration Services: SET_SYNC_ABS
                    break;
                case 0x04:
                    printf("SET_SYNC_REL\n"); // TODO Response Configuration Services: SET_SYNC_REL
                    break;
                case 0x05:
                    printf("GET_SYNC_CLOCK\n"); // TODO Response Configuration Services: GET_SYNC_CLOCK
                    break;
                case 0x09:
                    printf("SET_FILTER\n"); // TODO Response Configuration Services: SET_FILTER
                    break;
                case 0x0A:
                    printf("SET_FUNCTION\n");
                    if (msg[1] == 0xFFFF){
                        printf("\tinvalid request\n");
                    }else{
                        printf("\tSector number: %i\n",msg[1]);
                        switch (msg[2]) {
                            case 0:
                                printf("\tSector func: Not initialised\n");
                                break;
                            case 1:
                                printf("\tSector func: No measurement\n");
                                break;
                            case 3:
                                printf("\tSector func: Normal measurement\n");
                                break;
                            case 4:
                                printf("\tSector func: Reference measurement\n");
                                break;
                            default:
                                printf("\tSector func: invalid\n");
                        }
                        printf("\tSector stop: %f°\n", (float)msg[3] / 16);
                    }
                    break;
                case 0x0B:
                    printf("GET_FUNCTION\n");
                    if (msg[1] == 0xFFFF){
                        printf("\tinvalid request\n");
                    }else{
                        printf("\tSector number: %i\n",msg[1]);
                        switch (msg[2]) {
                            case 0:
                                printf("\tSector func: Not initialised\n");
                                break;
                            case 1:
                                printf("\tSector func: No measurement\n");
                                break;
                            case 3:
                                printf("\tSector func: Normal measurement\n");
                                break;
                            case 4:
                                printf("\tSector func: Reference measurement\n");
                                break;
                            default:
                                printf("\tSector func: invalid\n");
                        }
                        printf("\tSector stop: %f°\n", (float)msg[3] / 16);
                    }
                    break;

                default:
                    printf("\n");
                    break;
            }
            break;
        case 0x83:
            printf("Response to Measurement Services\n"); // TODO Response Measurement Services
            switch (msg[0] & 0xff){
                case 0x01:
                    printf("GetProfile response WIP\n");
                    printProfile(msg);

                    break;
                case 0x02:
                    printf("\tCANCEL_PROFILE\n");
                    printSenStat(msg[2], singleIndent);
                    break;
            }
            break;
        case 0x84:
            printf("Response to Working Services\n"); // TODO Response Working Services
            switch (msg[0] & 0xff)
            {
                case 0x01:
                    switch (msg[1])
                    {
                        case 0x0:
                            printf("\tReset (CPU reinitialized)\n");
                            break;
                        case 0x1:
                            printf("\tReset (CPU not reinitialized)\n");
                            break;
                        case 0x2:
                            printf("\tHalt application and enter IDLE state\n");
                            break;

                        default:
                            printf("invalid");
                            break;
                    }
                    break;
                case 0x2:
                    printf("\tEntered IDLE Mode\n");
                    printSenStat(msg[2], singleIndent);
                    break;

                case 0x3:
                    printf("\tEntered ROTATE Mode\n");
                    printSenStat(msg[2], singleIndent);
                    break;
                case 0x4:
                    printf("\tEntered MEASURE Mode\n");
                    printSenStat(msg[2], singleIndent);
                    printf("\terrorcode: %04X\n",msg[3]);
                    break;

                default:
                    break;
            }
            break;
        case 0x86:
            printf("Response to Interface Routing Services\n"); // TODO Response Interface Routing Services
            break;
        case 0x87:
            printf("Response to  File Services\n"); // TODO Response File Services
            break;
        case 0x88:
            printf("Response to Monitor Services\n");
            switch (msg[0] & 0xff) {
                case 0x01:
                    switch (msg[1]) {
                        case 0x0000:
                            printf("\tMONITOR_RUN request successful\n");
                            break;
                        case 0xFFFF:
                            printf("\tMONITOR_RUN request not successful\n");
                            break;
                        default:
                            printf("invalid\n");

                    }
                    break;
                case 0x02:
                    switch (msg[1]) {
                        case 0x0000:
                            printf("\tMONITOR_PROFILE_LOG request successful\n");
                            break;
                        case 0xFFFF:
                            printf("\tMONITOR_PROFILE_LOG request not successful\n");
                            break;
                        default:
                            printf("\tinvalid\n");

                    }
                    break;
                default:
                    printf("\tunknown MONITOR response.\n");
            }
            break;
        case 0xff: // TODO Response Special Services needs checking
            printf("Response to  Special Services\n");
            if ((msg[1] & 0xff) == 0)
            {
                printf("Response to  SERVICE_FAILURE\n");
                printSenStat(msg[4], singleIndent);
            }
            break;

        default:
            break;
    }
}

void printProfile(const vector<__uint16_t> &msg) { //TODO add multi sector support
    WORD profileFormat=msg[1];
    char numberOfSectors= (char)(msg[2] & 0xFF);
    char numberOfLayers= (char)(msg[2] >> 8);
    int msgData= 3;
    unsigned long offset;
    int pointsPerSector=-1;
    bool distance=profileFormat&0b100000000, direction=profileFormat&0b1000000000, echo=profileFormat&0b10000000000;

    printf("\tProfileFormat: %04X\n",profileFormat);
    printf("\tNumber of Layers: %i\n",numberOfLayers);
    printf("\tNumber of Sectors: %i\n",numberOfSectors);
    if (profileFormat&0b1){ // bit 0
        printf("\tProfile sent: %i \n",msg[msgData]);
    }
    if (profileFormat&0b10){ // bit 1
        offset = bitset<16>(profileFormat&1).count();
        printf("\tProfile count: %i\n",msg[msgData+(offset)]);
    }
    if (profileFormat&0b100){ // bit 2
        offset = bitset<16>(profileFormat&0b11).count();
        printf("\tcurrentLayer: %i\n",msg[msgData+(offset)]);
    }
    if (profileFormat&0b1000){ //bit 3
        offset = bitset<16>(profileFormat&0b111).count();
        printf("\tSector number: %i\n",msg[msgData+(offset)]);
    }
    if (profileFormat&0b10000){ // bit 4
        offset = bitset<16>(profileFormat&0b1111).count();
        printf("\tAngle Step: %f\n",(float)msg[msgData+(offset)]/16);
    }
    if (profileFormat&0b100000){ // bit 5
        offset = bitset<16>(profileFormat&0b11111).count();
        pointsPerSector=msg[msgData+(offset)];
        printf("\tNumber of Points in Sector: %i\n",pointsPerSector);
    }
    if (profileFormat&0b1000000){ // bit 6
        offset = bitset<16>(profileFormat&0b111111).count();
        printf("\tTimestamp start Sector: %i\n",msg[msgData+(offset)]);
    }
    if (profileFormat&0b10000000){ // bit 7
        offset = bitset<16>(profileFormat&0b1111111).count();
        printf("\tstart Angle: %f°\n",(float)msg[msgData+(offset)]/16);
    }
    if (distance||direction||echo){ // bit 8 9 10
        offset = bitset<16>(profileFormat&0xFF).count();
        if (pointsPerSector!=-1){
            printf("\tpoints: ",offset);
            for (int iPoints = 0; iPoints < pointsPerSector; ++iPoints) {
                unsigned long iterationOffset = offset+(distance+direction+echo)*iPoints;
                printf("(",iPoints,iterationOffset);
                if (distance) {
                    printf("%.4f",(float)msg[msgData+iterationOffset]/256);
                }
                if (direction) {
                    printf(",%.4f",(float)msg[msgData+iterationOffset+distance]/16);
                }
                if (echo) {
                    printf(",%i",msg[msgData+iterationOffset+distance+direction]);
                }

                printf("\t)");
            }
            printf("\n");
        }
    }
    // TEND, ENDDRIR, SENSTAT mayby von len aus
}

void printSenStat(WORD senStat, char* indent){

    printf("%sSenStat: %04X\n",indent,senStat);
    printf("%s\t Working Mode: ",indent);
    switch (senStat&0xF)
    {
        case 0x1:
            printf("IDLE Mode\n");
            break;

        case 0x2:
            printf("ROTATE Mode\n");
            break;
        case 0x3:
            printf("MEASURE Mode\n");
            break;
        case 0x4:
            printf("ERROR Mode\n");
            break;

        default:
            printf("invalid: %X\n",senStat);
            break;
    }
    printf("%s\t Motor Mode: ",indent);
    switch ((senStat>>4)&0xF)
    {
        case 0x0:
            printf("Motor Ok\n");
            break;
        case 0x9:
            printf("Motor spin to high\n");
            break;
        case 0xA:
            printf("Motor spin to low\n");
            break;
        case 0xB:
            printf("Motor stops or coder error\n");
            break;

        default:
            printf("invalid: %X\n",senStat);
            break;
    }

}

void printPortVal(WORD portVal, char* indent){
    printf("%sPort status: \n",indent);
    printf("%s\tLED 0(yellow):\t%i\n", indent, portVal & 1);
    printf("%s\tLED 1(yellow):\t%i\n",indent, (portVal >> 1) & 1);
    printf("%s\tLED 2(green):\t%i\n",indent, (portVal >> 2) & 1);
    printf("%s\tLED 3(red):\t%i\n",indent, (portVal >> 3) & 1);
    printf("%s\tSwitch 0:\t%i\n",indent, (portVal >> 4) & 1);
    printf("%s\tSwitch 1:\t%i\n",indent, (portVal >> 5) & 1);
    printf("%s\tSwitch 2:\t%i\n",indent, (portVal >> 6) & 1);
    printf("%s\tSwitch 3:\t%i\n",indent, (portVal >> 7) & 1);

}