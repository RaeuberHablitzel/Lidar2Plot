// Tutorial found at: https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/

#include "comtest.h"



using namespace std;

// TODO clean up
// calculates an array of words from a given msg_string, returns the length of res
// int parseMsg(msg_String, length of msg, pointer to result, idx of result)
int convertMsgToWords(vector<char> &msg,  vector<__uint16_t> &res)
{
    __uint16_t tempRes[buf_size];
    int len= msg.size();
    res.clear();
    // printf("msg:%s\n",msg);
    int i = 0;
    for (int a = 1; a < (len - 2); a += 2)
    {
        char hexSubString[3] = {msg[a], msg[a + 1], '\0'};
        __uint16_t number = strtol(hexSubString, nullptr, 16);
        // printf("a:%i\tread: %s\tas: %lx\n",a,hexSubString,number);
        tempRes[i++] = number;
    }
    printf("msg in words: ");//DEBUG

    for (i = 0; i < (len - 2) / 2; i += 2)
    {
        __uint16_t tmp = (tempRes[i] << 8) + tempRes[i + 1];
        res.push_back(tmp);
        printf("%04x,", tmp);//DEBUG
    }
    printf("\n");//DEBUG
    printf("msg as ascii: ");
    for (i = 0; i < (len - 2) / 2; i += 2)
    {
        printf(" %c %c,",std::min(std::max(0x20,(int)tempRes[i]), 0x7E),std::min(std::max(0x20,(int)tempRes[i+1]), 0x7E) );
    }
    printf("\n");
    return (len - 2) / 4;
}

bool readBuffer(const char *readBuf, vector<char> &msgBuf, int bytesRead, bool continueLastMsg,vector<__uint16_t> &telegramBuffer)
{

    int i = 0;
    bool msgStartFound = continueLastMsg;
    // bool msgEndFound = false;
    while (i < bytesRead)
    {
        if (readBuf[i] == 0x15)
        {
            printf("%serror Msg%s\n",printColorRed,printColorNormal);
        }
        if (readBuf[i] == 0x02 || msgStartFound)
        {
            msgStartFound = true;
            while (i < bytesRead )
            {
                if (readBuf[i] == 0x02)
                {
                    msgBuf.clear();
                }
                msgBuf.push_back(readBuf[i]);
                if (readBuf[i] == 0x03)
                {
                    // printf("msg:%s\n",msgBuf);
                    vector<__uint16_t> result;
                    convertMsgToWords(msgBuf, result);

                    parseMsg(result, telegramBuffer); // do something with result

                    // DEBUG print result
                    /*printf("pas in words: ");
                    for (int k= 0; k<l; ++k){
                        printf("%04lx,",result[k]);
                    }
                    printf("\nl:%i,j:%i\n",l,j);*/

                    msgStartFound = false;

                    i++;
                    break;
                }
                i++;
            }
        }
        else
            i++;
    }
    return  msgStartFound;
}

WORD block_crc16_word(WORD *data, WORD numOfBytes, WORD initial_crc)
{
    WORD d;
    WORD crc = initial_crc;
    numOfBytes >>= 1;
    while (numOfBytes--)
    {
        d = *data++;
        crc = ((crc << 8) | ((BYTE)(d >> 8))) ^ crctab[crc >> 8];
        crc = ((crc << 8) | ((BYTE)d)) ^ crctab[crc >> 8];
    }
    return crc;
}

void buildMsgFromWord(const WORD *data, WORD dataLen, WORD (&msg)[])
{
    // WORD msg[dataLen+3];
    memset(&msg, 0, dataLen + 3);
    msg[0] = 0x1410;
    msg[1] = dataLen + 1;
    for (int i = 0; i < dataLen; ++i)
    {
        msg[i + 2] = data[i];
    }
    msg[dataLen + 2] = block_crc16_word(msg, (dataLen + 2) * 2, 0xFFFF);
    /*for (int i=0;i<dataLen+3;++i){
      //msg[i]>>8,msg[i]&0xff
    }*/
}

void sendCommand(int serial_port, WORD *data, WORD dataLen)
{
    WORD msg[dataLen + 3];
    buildMsgFromWord(data, dataLen, reinterpret_cast<__uint16_t (&)[]>(msg));
    std::string telegram;
    telegram.append({0x2});
    printf("Msg to Send: ");
    for (int i = 0; i < dataLen + 3; ++i)
    {
        telegram.append(wordToString(msg[i]));
        printf("%04X,",msg[i]);
    }
    telegram.append({0x3});

    write(serial_port, telegram.c_str(), telegram.size());
    printf("\n");
}

std::string wordToString(WORD input)
{
    std::string res;
    for (char i = 3; i >= 0; --i)
    {
        char t = (input >> (i * 4)) & 0xF;
        res.append({(char)(t + 0x30 + (t > 9) * 0x07)});
    }
    return res;
}

void parseMsg(vector<WORD> &msg,  vector<__uint16_t> &telegramBuffer)
{
    int len= msg.size();
    // check crc
    WORD crc = block_crc16_word(&msg[0], (len - 1) * 2, 0xffff);
    if (crc != msg[len - 1])
    {
        printf("%scrc readError:\n\texpected crc: %04X, found instead: %04X%s\n",printColorRed, crc, msg[len - 1],printColorNormal);
        return;
    }

    // DEBUG DATA
    printf("crcToCheck: %04X\n", msg[len - 1]);
    printf("crcCalculated: %04X\n", msg[len - 1]);
    printf("sendAddress: %04X\n", msg[0]);
    printf("sendMsgLen: %04X\n", msg[1]);

    if (len > 3 && msg[2] == 0)
    { // check if single package   //TODO make separate fun/class
        printResponse(msg,3);
    }
    else
    {
        printf("parsing multi pkg msg\n");

        if(msg[2]==0xFFFF){
            telegramBuffer.clear();
            printf("multi pkg start\n");
        }
        for (int i = 3+(msg[2]==0xFFFF);i<len-1;++i){
            telegramBuffer.push_back(msg[i]);
        }
        if (msg[2]==0x1){
            printf("multi pkg end\n");
            printResponse(telegramBuffer,0);//do something with telegramBuffer;

        }
    }

}

void printResponse(vector<WORD> &msg,int pkgHeaderIdx){

    char singleIndent[]= "\t";
    // translate header
    //  high-byte as Service group
    //  loy-byte as Service
    switch ((msg[pkgHeaderIdx] & 0xff00) >> 8)
    {
        case 0x81:
            printf("Response to Status Service: ");
            switch (msg[pkgHeaderIdx] & 0xff)
            {
                case 0x01:
                    printf("GET_IDENTIFICATION \n\t");
                    for (int i=pkgHeaderIdx+1; i<msg[1]-2;++i){
                        printf("%c%c",(msg[i]>>8),msg[i]&0xFF);
                    }
                    printf("\n");
                    printSenStat(msg[pkgHeaderIdx+8],singleIndent);

                    break;
                case 0x02:
                    printf("GET_STATUS\n");
                    printSenStat(msg[pkgHeaderIdx+2],singleIndent);
                    break;
                case 0x04:
                    printf("GET_SIGNAL\n");
                    printPortval(msg[pkgHeaderIdx+1],singleIndent);
                    break;
                case 0x05:
                    printf("SET_SIGNAL\n");
                    if (msg[pkgHeaderIdx+1]==0xFFFF){
                        printf("\tfailed to set signals");
                    }else{
                        printPortval(msg[pkgHeaderIdx+1],singleIndent);
                    }
                    break;
                case 0x6:
                    printf("REGISTER_APPLICATION\n");
                    printf("\tRegistration ");
                    if(!msg[pkgHeaderIdx+1]){
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
            switch (msg[pkgHeaderIdx] & 0xff)
            {
                case 0x01:
                    printf("SET_CONFIG\n"); // TODO Response Configuration Services: SET_CONFIG

                    switch (msg[pkgHeaderIdx+1]) {
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
                    switch (msg[pkgHeaderIdx + 1])
                    {
                        case 0x0001: // RS232/422
                            printf("\tRS232 / RS422 Config:\n\t\tBaud rate: ");
                            switch (msg[pkgHeaderIdx + 2]) // baud rate
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
                            switch (msg[pkgHeaderIdx + 3]) // parity
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
                            switch (msg[pkgHeaderIdx + 4]) // stop bits
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
                            if (msg[pkgHeaderIdx + 5] <= 8)
                            {
                                printf("%i\n", msg[pkgHeaderIdx + 5]);
                            }
                            else
                            {
                                printf("invalid\n");
                            }
                            break;
                        case 0x0002:
                            printf("\tCAN Config:\n\t\tdata transmission rate: ");
                            switch (msg[pkgHeaderIdx + 2]) // transmission rate
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
                            printf("\t\tBasic  value of the host CAN identifier: %04x\n", msg[pkgHeaderIdx + 3]);
                            printf("\t\tMask value of the host CAN identifier: %04x\n", msg[pkgHeaderIdx + 4]);
                            printf("\t\tBasic value of the LD-OEM/LD-LRS CAN identifier: %04x\n", msg[pkgHeaderIdx + 5]);
                            printf("\t\tBroadcast-ID (0xFFFF if disabled): %04x\n", msg[pkgHeaderIdx + 6]);
                            break;
                        case 0x0005:
                            printf("\tEthernet Config:\n");
                            printf("\t\tIP Address:\t%i.%i.%i.%i\n",msg[pkgHeaderIdx+2],msg[pkgHeaderIdx+3],msg[pkgHeaderIdx+4],msg[pkgHeaderIdx+5]);
                            printf("\t\tSubnet mask:\t%i.%i.%i.%i\n",msg[pkgHeaderIdx+6],msg[pkgHeaderIdx+7],msg[pkgHeaderIdx+8],msg[pkgHeaderIdx+9]);
                            printf("\t\tStandard gateway:\t%i.%i.%i.%i\n",msg[pkgHeaderIdx+10],msg[pkgHeaderIdx+11],msg[pkgHeaderIdx+12],msg[pkgHeaderIdx+13]);
                            printf("\t\tNode ID: %04x\n",msg[pkgHeaderIdx+14]);
                            printf("\t\tTCP/IP Transparent port: %04x\n",msg[pkgHeaderIdx+15]);

                            break;
                        case 0x0010: // Global config
                            printf("\tGlobal Config:\n\t\tSensorID: ");
                            if (msg[pkgHeaderIdx+2]<=254){
                                printf("%02x\n",msg[pkgHeaderIdx+2]);
                            }else{
                                printf("invalid\n");
                            }
                            printf("\t\tNominal motor speed: ");
                            if (msg[pkgHeaderIdx+3]<=20 && msg[pkgHeaderIdx+3]>=5){
                                printf("%iHz\n",msg[pkgHeaderIdx+3]);
                            }else{
                                printf("invalid\n");
                            }
                            printf("\t\tAngle step: ");
                            if (msg[pkgHeaderIdx+4]>=1 ){ //TODO divisor
                                float f=((float)msg[pkgHeaderIdx+4])/16;
                                printf("%f° raw: %i\n",f,msg[pkgHeaderIdx+4]);
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
                    if (msg[pkgHeaderIdx+1]==0xFFFF){
                        printf("\tinvalid request\n");
                    }else{
                        printf("\tSector number: %i\n",msg[pkgHeaderIdx+1]);
                        switch (msg[pkgHeaderIdx+2]) {
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
                        printf("\tSector stop: %f°\n",(float)msg[pkgHeaderIdx+3]/16);
                    }
                    break;
                case 0x0B:
                    printf("GET_FUNCTION\n");
                    if (msg[pkgHeaderIdx+1]==0xFFFF){
                        printf("\tinvalid request\n");
                    }else{
                        printf("\tSector number: %i\n",msg[pkgHeaderIdx+1]);
                        switch (msg[pkgHeaderIdx+2]) {
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
                        printf("\tSector stop: %f°\n",(float)msg[pkgHeaderIdx+3]/16);
                    }
                    break;

                default:
                    printf("\n");
                    break;
            }
            break;
        case 0x83:
            printf("Response to Measurement Services\n"); // TODO Response Measurement Services
            switch (msg[pkgHeaderIdx] & 0xff){
                case 0x01:
                    printf("GetProfile response WIP\n");
                    printProfile(msg, pkgHeaderIdx);

                    break;
                case 0x02:
                    printf("\tCANCEL_PROFILE\n");
                    printSenStat(msg[pkgHeaderIdx+2],singleIndent);
                    break;
            }
            break;
        case 0x84:
            printf("Response to Working Services\n"); // TODO Response Working Services
            switch (msg[pkgHeaderIdx] & 0xff)
            {
                case 0x01:
                    switch (msg[pkgHeaderIdx+1])
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
                    printSenStat(msg[pkgHeaderIdx+2],singleIndent);
                    break;

                case 0x3:
                    printf("\tEntered ROTATE Mode\n");
                    printSenStat(msg[pkgHeaderIdx+2],singleIndent);
                    break;
                case 0x4:
                    printf("\tEntered MEASURE Mode\n");
                    printSenStat(msg[pkgHeaderIdx+2],singleIndent);
                    printf("\terrorcode: %04X\n",msg[pkgHeaderIdx+3]);
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
            switch (msg[pkgHeaderIdx] & 0xff) {
                case 0x01:
                    switch (msg[pkgHeaderIdx+1]) {
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
                    switch (msg[pkgHeaderIdx+1]) {
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
            if ((msg[pkgHeaderIdx+1] & 0xff) == 0)
            {
                printf("Response to  SERVICE_FAILURE\n");
                printSenStat(msg[pkgHeaderIdx+4],singleIndent);
            }
            break;

        default:
            break;
    }
}

// prints a given Profile
void printProfile(const vector<__uint16_t> &msg, int pkgHeaderIdx) { //TODO add multi sector support
        WORD profileFormat=msg[pkgHeaderIdx + 1];
    char numberOfSectors= (char)(msg[pkgHeaderIdx+2]&0xFF);
    char numberOfLayers= (char)(msg[pkgHeaderIdx+2]>>8);
    int msgData=pkgHeaderIdx+3;
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


void printPortval(WORD portval, char* indent){
        printf("%sPort status: \n",indent);
        printf("%s\tLED 0(yellow):\t%i\n",indent,portval&1);
        printf("%s\tLED 1(yellow):\t%i\n",indent,(portval>>1)&1);
        printf("%s\tLED 2(green):\t%i\n",indent,(portval>>2)&1);
        printf("%s\tLED 3(red):\t%i\n",indent,(portval>>3)&1);
        printf("%s\tSwitch 0:\t%i\n",indent,(portval>>4)&1);
        printf("%s\tSwitch 1:\t%i\n",indent,(portval>>5)&1);
        printf("%s\tSwitch 2:\t%i\n",indent,(portval>>6)&1);
        printf("%s\tSwitch 3:\t%i\n",indent,(portval>>7)&1);
        
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

void configureSerialPort(int serial_port)
{

    // Create new termios struct, we call it 'tty' for convention
    // No need for "= {0}" at the end as we'll immediately write the existing
    // config to this struct
    struct termios tty;

    // Read in existing settings, and handle any error
    // NOTE: This is important! POSIX states that the struct passed to tcsetattr()
    // must have been initialized with a call to tcgetattr() otherwise behaviour
    // is undefined

    if (tcgetattr(serial_port, &tty) != 0)
    {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
    }

    tty.c_cflag &= ~PARENB;        // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB;        // Clear stop field, only one stop bit is used in communication (most common)
    tty.c_cflag |= CS8;            // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS;       // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;   // Disable echo
    tty.c_lflag &= ~ECHOE;  // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG;   // Disable interpretation of INTR, QUIT and SUSP

    // Input modes
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);                                      // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes

    // Output Modes
    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

    // read behavior
    tty.c_cc[VTIME] = 5; // Wait for up to 1s (10 deci seconds), returning as soon as any data is received.
    // tty.c_cc[VTIME] = 0;
    tty.c_cc[VMIN] = 100;

    // Set in/out baud rate to be 115200

    cfsetspeed(&tty, baud_rate);

    // Save tty settings, also checking for error
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
    {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    }
}

int inputMsg(int SerialPort){ //TODO add auto split msg
    string inputString;
    int repeat=1;
    regex wordRegex("^([A-Fa-f0-9]{4},)*([A-Fa-f0-9]{4},?$)");
    cout << "enter Command: ";
    cin >> inputString;

    if (inputString == "q" || inputString == "Q")
        repeat=0;
    else {
        if(regex_match(inputString, wordRegex)){
            inputString.erase(remove(inputString.begin(),inputString.end(),','),inputString.end());
            inputString.insert(0,"q0000");
            inputString.append("q");

            vector<__uint16_t> res;

            vector<char> charInput(inputString.begin(),inputString.end());
            int l= convertMsgToWords(charInput,res);
            sendCommand(SerialPort,&res[0],l);
        }else{
            cout << "invalid Command" << endl;
           repeat= -1;
        }

    }
    return repeat;
}

int main()
{
    int serial_port = open(com_port, O_RDWR);
    // Check for errors
    if (serial_port < 0)
    {
        printf("Error %i from open: %s\n", errno, strerror(errno));
    }

    configureSerialPort(serial_port);


    // writing stuff
    // 1410h 0004h 0000h 0202h 0001h 2BBCh
    //unsigned char msg[] = {0x02, '1', '4', '1', '0', '0', '0', '0', '4', '0', '0', '0', '0', '0', '2', '0', '2', '0', '0', '0', '1', '2', 'B', 'B', 'C', 0x03};
    // write(serial_port, msg, sizeof(msg));

    //WORD testMsg[] = {0x0000, 0x0202, 0x0010};
    //WORD testMsg[] = {0x0000, 0x0202,0x005};
    //sendCommand(serial_port, testMsg,3);
    
    //WORD testMsg2[] = {0x0000, 0x8202, 0x0001, 0x0006, 0x0000, 0x0001, 0x0008};
    // sendCommand(serial_port,testMsg2,7);
    int opMode=0;
    do {
        cout << "Select op Mode: read only (1), write only(2), read/write(3): ";
        cin >> opMode;
    } while (opMode<1||opMode>3);

    vector<__uint16_t> telegramBuffer;
    vector<char> msg_buf;

    bool incompleteMsg = false;
    // Allocate memory for read buffer, set size according to your needs
    char read_buf[buf_size];
    int bytes_available=0;
    int repeat= 1;
    while(repeat){
        if (opMode&2)
            repeat= inputMsg(serial_port);

        if (repeat>0 && (opMode&1)) {
            cout << "waiting for response" <<endl;
            while (bytes_available == 0) {
                ioctl(serial_port, FIONREAD, &bytes_available);
            }
            do {
                // Normally you wouldn't do this memset() call, but since we will just receive
                // ASCII data for this example, we'll set everything to 0, so we can
                // call printf() easily.
                memset(&read_buf, '\0', sizeof(read_buf)); // maybe not needed

                // Read bytes. The behaviour of read() (e.g. does it block?,
                // how long does it block for?) depends on the configuration
                // settings above, specifically VMIN and VTIME
                int num_bytes = read(serial_port, &read_buf, sizeof(read_buf));

                // n is the number of bytes read. n may be 0 if no bytes were received, and can also be -1 to signal an error.
                if (num_bytes < 0) {
                    printf("Error reading: %s", strerror(errno));
                    return 1;
                }





                incompleteMsg = readBuffer(read_buf, msg_buf, num_bytes, incompleteMsg, telegramBuffer);
                ioctl(serial_port, FIONREAD, &bytes_available);
            } while (incompleteMsg || bytes_available > 0);
        }
    }
    printf("returned: %i\n", incompleteMsg);



    close(serial_port);
    return 0;
}