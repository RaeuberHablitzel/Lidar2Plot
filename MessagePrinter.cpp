//
// Created by eric on 30.03.22.
//

#include "MessagePrinter.h"


using namespace std;

void printResponse(vector<WORD> &msg) {
    char printBuffer[printBuffer_size];
    snprintf(printBuffer,printBuffer_size-strlen(printBuffer),"\nheader: %04X\n",msg[0]);
    char singleIndent[]= "\t";
    // translate header
    //  high-byte as Service group
    //  loy-byte as Service
    switch ((msg[0] & 0xff00) >> 8)
    {
        case 0x81:
            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"Response to Status Service: ");
            switch (msg[0] & 0xff)
            {
                case 0x01:
                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"GET_IDENTIFICATION \n\t");
                    for (int i= 1; i < msg[1] - 2; ++i){
                        snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"%c%c",(msg[i]>>8),msg[i]&0xFF);
                    }
                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\n");
                    printSenStat(msg[8], singleIndent, printBuffer);

                    break;
                case 0x02:
                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"GET_STATUS\n");
                    printSenStat(msg[2], singleIndent, printBuffer);
                    break;
                case 0x04:
                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"GET_SIGNAL\n");
                    printPortVal(msg[1], singleIndent, printBuffer);
                    break;
                case 0x05:
                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"SET_SIGNAL\n");
                    if (msg[1] == 0xFFFF){
                        snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tfailed to set signals");
                    }else{
                        printPortVal(msg[1], singleIndent, printBuffer);
                    }
                    break;
                case 0x6:
                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"REGISTER_APPLICATION\n");
                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tRegistration ");
                    if(!msg[1]){
                        snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"successful\n");
                    }else{
                        snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"failed\n");
                    }
                    break;

                default:
                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\n");
                    break;
            }
            break;
        case 0x82:
            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"Response to Configuration Services: ");
            switch (msg[0] & 0xff)
            {
                case 0x01:
                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"SET_CONFIG\n"); // TODO Response Configuration Services: SET_CONFIG

                    switch (msg[1]) {
                        case 0x0000:
                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"request successful\n");
                            break;
                        case 0xFFFF:
                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"request not successful\n");
                            break;
                        default:
                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"invalid\n");

                    }
                    break;
                case 0x02:
                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"GET_CONFIG\n");
                    switch (msg[1])
                    {
                        case 0x0001: // RS232/422
                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tRS232 / RS422 Config:\n\t\tBaud rate: ");
                            switch (msg[2]) // baud rate
                            {
                                case 0x0001:
                                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"4800 bd\n");
                                    break;
                                case 0x0002:
                                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"9600 bd\n");
                                    break;
                                case 0x0003:
                                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"19200 bd\n");
                                    break;
                                case 0x0004:
                                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"38400 bd\n");
                                    break;
                                case 0x0005:
                                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"57600 bd\n");
                                    break;
                                case 0x0006:
                                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"115200 bd\n");
                                    break;

                                default:
                                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"unknown/invalid\n");
                                    break;
                            }

                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\t\tParity: ");
                            switch (msg[3]) // parity
                            {
                                case 0x0000:
                                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"No parity\n");
                                    break;
                                case 0x0001:
                                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"Even parity\n");
                                    break;
                                case 0x0002:
                                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"Odd parity\n");
                                    break;

                                default:
                                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"unknown/invalid\n");
                                    break;
                            }

                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\t\tNumber of Stop bits: ");
                            switch (msg[4]) // stop bits
                            {
                                case 0x0001:
                                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"1\n");
                                    break;
                                case 0x0002:
                                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"2\n");
                                    break;

                                default:
                                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"unknown/invalid\n");
                                    break;
                            }
                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\t\tBits per character: ");
                            if (msg[5] <= 8)
                            {
                                snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"%i\n", msg[5]);
                            }
                            else
                            {
                                snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"invalid\n");
                            }
                            break;
                        case 0x0002:
                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tCAN Config:\n\t\tdata transmission rate: ");
                            switch (msg[2]) // transmission rate
                            {
                                case 0x0000:
                                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"10 kBit/s\n");
                                    break;
                                case 0x0001:
                                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"20 kBit/s\n");
                                    break;
                                case 0x0002:
                                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"50 kBit/s\n");
                                    break;
                                case 0x0003:
                                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"125 kBit/s\n");
                                    break;
                                case 0x0004:
                                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"250 kBit/s\n");
                                    break;
                                case 0x0005:
                                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"500 kBit/s\n");
                                    break;
                                case 0x0006:
                                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"1 MBit/s\n");
                                    break;

                                default:
                                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"unknown/invalid\n");
                                    break;
                            }
                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\t\tBasic  value of the host CAN identifier: %04x\n", msg[3]);
                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\t\tMask value of the host CAN identifier: %04x\n", msg[4]);
                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\t\tBasic value of the LD-OEM/LD-LRS CAN identifier: %04x\n", msg[5]);
                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\t\tBroadcast-ID (0xFFFF if disabled): %04x\n", msg[6]);
                            break;
                        case 0x0005:
                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tEthernet Config:\n");
                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\t\tIP Address:\t%i.%i.%i.%i\n", msg[2], msg[3], msg[4], msg[5]);
                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\t\tSubnet mask:\t%i.%i.%i.%i\n", msg[6], msg[7], msg[8], msg[9]);
                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\t\tStandard gateway:\t%i.%i.%i.%i\n", msg[10], msg[11], msg[12], msg[13]);
                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\t\tNode ID: %04x\n",msg[14]);
                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\t\tTCP/IP Transparent port: %04x\n",msg[15]);

                            break;
                        case 0x0010: // Global config
                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tGlobal Config:\n\t\tSensorID: ");
                            if (msg[2] <= 254){
                                snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"%02x\n",msg[2]);
                            }else{
                                snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"invalid\n");
                            }
                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\t\tNominal motor speed: ");
                            if (msg[3] <= 20 && msg[3] >= 5){
                                snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"%iHz\n",msg[3]);
                            }else{
                                snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"invalid\n");
                            }
                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\t\tAngle step: ");
                            if (msg[4] >= 1 ){ //TODO divisor
                                float f= ((float)msg[4]) / 16;
                                snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"%f° raw: %i\n",f,msg[4]);
                            }else{
                                snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"invalid\n");
                            }


                            break;

                        default:
                            break;
                    }
                    break;
                case 0x03:
                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"SET_SYNC_ABS\n"); // TODO Response Configuration Services: SET_SYNC_ABS
                    break;
                case 0x04:
                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"SET_SYNC_REL\n"); // TODO Response Configuration Services: SET_SYNC_REL
                    break;
                case 0x05:
                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"GET_SYNC_CLOCK\n"); // TODO Response Configuration Services: GET_SYNC_CLOCK
                    break;
                case 0x09:
                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"SET_FILTER\n"); // TODO Response Configuration Services: SET_FILTER
                    break;
                case 0x0A:
                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"SET_FUNCTION\n");
                    if (msg[1] == 0xFFFF){
                        snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tinvalid request\n");
                    }else{
                        snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tSector number: %i\n",msg[1]);
                        switch (msg[2]) {
                            case 0:
                                snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tSector func: Not initialised\n");
                                break;
                            case 1:
                                snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tSector func: No measurement\n");
                                break;
                            case 3:
                                snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tSector func: Normal measurement\n");
                                break;
                            case 4:
                                snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tSector func: Reference measurement\n");
                                break;
                            default:
                                snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tSector func: invalid\n");
                        }
                        snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tSector stop: %f°\n", (float)msg[3] / 16);
                    }
                    break;
                case 0x0B:
                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"GET_FUNCTION\n");
                    if (msg[1] == 0xFFFF){
                        snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tinvalid request\n");
                    }else{
                        snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tSector number: %i\n",msg[1]);
                        switch (msg[2]) {
                            case 0:
                                snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tSector func: Not initialised\n");
                                break;
                            case 1:
                                snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tSector func: No measurement\n");
                                break;
                            case 3:
                                snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tSector func: Normal measurement\n");
                                break;
                            case 4:
                                snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tSector func: Reference measurement\n");
                                break;
                            default:
                                snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tSector func: invalid\n");
                        }
                        snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tSector stop: %f°\n", (float)msg[3] / 16);
                    }
                    break;

                default:
                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\n");
                    break;
            }
            break;
        case 0x83:
            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"Response to Measurement Services\n"); // TODO Response Measurement Services
            switch (msg[0] & 0xff){
                case 0x01:
                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"GetProfile response WIP\n");
                    printProfile(msg, printBuffer);

                    break;
                case 0x02:
                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tCANCEL_PROFILE\n");
                    printSenStat(msg[2], singleIndent, printBuffer);
                    break;
            }
            break;
        case 0x84:
            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"Response to Working Services\n"); // TODO Response Working Services
            switch (msg[0] & 0xff)
            {
                case 0x01:
                    switch (msg[1])
                    {
                        case 0x0:
                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tReset (CPU reinitialized)\n");
                            break;
                        case 0x1:
                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tReset (CPU not reinitialized)\n");
                            break;
                        case 0x2:
                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tHalt application and enter IDLE state\n");
                            break;

                        default:
                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"invalid");
                            break;
                    }
                    break;
                case 0x2:
                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tEntered IDLE Mode\n");
                    printSenStat(msg[2], singleIndent, printBuffer);
                    break;

                case 0x3:
                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tEntered ROTATE Mode\n");
                    printSenStat(msg[2], singleIndent, printBuffer);
                    break;
                case 0x4:
                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tEntered MEASURE Mode\n");
                    printSenStat(msg[2], singleIndent, printBuffer);
                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\terrorcode: %04X\n",msg[3]);
                    break;

                default:
                    break;
            }
            break;
        case 0x86:
            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"Response to Interface Routing Services\n"); // TODO Response Interface Routing Services
            break;
        case 0x87:
            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"Response to  File Services\n"); // TODO Response File Services
            break;
        case 0x88:
            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"Response to Monitor Services\n");
            switch (msg[0] & 0xff) {
                case 0x01:
                    switch (msg[1]) {
                        case 0x0000:
                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tMONITOR_RUN request successful\n");
                            break;
                        case 0xFFFF:
                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tMONITOR_RUN request not successful\n");
                            break;
                        default:
                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"invalid\n");

                    }
                    break;
                case 0x02:
                    switch (msg[1]) {
                        case 0x0000:
                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tMONITOR_PROFILE_LOG request successful\n");
                            break;
                        case 0xFFFF:
                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tMONITOR_PROFILE_LOG request not successful\n");
                            break;
                        default:
                            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tinvalid\n");

                    }
                    break;
                default:
                    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tunknown MONITOR response.\n");
            }
            break;
        case 0xff: // TODO Response Special Services needs checking
            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"Response to  Special Services\n");
            if ((msg[1] & 0xff) == 0)
            {
                snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"Response to  SERVICE_FAILURE\n");
                printSenStat(msg[4], singleIndent, printBuffer);
            }
            break;

        default:
            break;
    }
    printf("%s",printBuffer);
}

void printProfile(const vector<__uint16_t> &msg, char* printBuffer) { //TODO add multi sector support
    WORD profileFormat=msg[1];
    char numberOfSectors= (char)(msg[2] & 0xFF);
    char numberOfLayers= (char)(msg[2] >> 8);
    int msgData= 3;
    unsigned long offset;
    int pointsPerSector=-1;
    bool distance=profileFormat&0b100000000, direction=profileFormat&0b1000000000, echo=profileFormat&0b10000000000;

    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tProfileFormat: %04X\n",profileFormat);
    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tNumber of Layers: %i\n",numberOfLayers);
    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tNumber of Sectors: %i\n",numberOfSectors);
    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tdist: %i,dir: %i,echo: %i\n",distance,direction,echo);

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
    }
    if (profileFormat&0b10000){ // bit 4
        offset = bitset<16>(profileFormat&0b1111).count();
        snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tAngle Step: %f\n",(float)msg[msgData+(offset)]/16);
    }
    if (profileFormat&0b100000){ // bit 5
        offset = bitset<16>(profileFormat&0b11111).count();
        pointsPerSector=msg[msgData+(offset)];
        snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"\tNumber of Points in Sector: %i\n",pointsPerSector);
    }
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
    }
    // TEND, ENDDRIR, SENSTAT mayby von len aus
}

void printSenStat(WORD senStat, char* indent, char* printBuffer){

    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"%sSenStat: %04X\n",indent,senStat);
    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"%s\t Working Mode: ",indent);
    switch (senStat&0xF)
    {
        case 0x1:
            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"IDLE Mode\n");
            break;

        case 0x2:
            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"ROTATE Mode\n");
            break;
        case 0x3:
            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"MEASURE Mode\n");
            break;
        case 0x4:
            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"ERROR Mode\n");
            break;

        default:
            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"invalid: %X\n",senStat);
            break;
    }
    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"%s\t Motor Mode: ",indent);
    switch ((senStat>>4)&0xF)
    {
        case 0x0:
            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"Motor Ok\n");
            break;
        case 0x9:
            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"Motor spin to high\n");
            break;
        case 0xA:
            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"Motor spin to low\n");
            break;
        case 0xB:
            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"Motor stops or coder error\n");
            break;

        default:
            snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"invalid: %X\n",senStat);
            break;
    }

}

void printPortVal(WORD portVal, char* indent, char* printBuffer){
    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"%sPort status: \n",indent);
    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"%s\tLED 0(yellow):\t%i\n", indent, portVal & 1);
    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"%s\tLED 1(yellow):\t%i\n",indent, (portVal >> 1) & 1);
    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"%s\tLED 2(green):\t%i\n",indent, (portVal >> 2) & 1);
    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"%s\tLED 3(red):\t%i\n",indent, (portVal >> 3) & 1);
    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"%s\tSwitch 0:\t%i\n",indent, (portVal >> 4) & 1);
    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"%s\tSwitch 1:\t%i\n",indent, (portVal >> 5) & 1);
    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"%s\tSwitch 2:\t%i\n",indent, (portVal >> 6) & 1);
    snprintf(printBuffer+ strlen(printBuffer), printBuffer_size-strlen(printBuffer),"%s\tSwitch 3:\t%i\n",indent, (portVal >> 7) & 1);

}