//
// Created by eric on 29.03.22.
//

#include "MessageHandler.h"
#include "common .h"
using namespace std;


void parseMsgRS232(vector<__uint16_t> &msg, vector<__uint16_t> &telegramBuffer, void *action (vector<__uint16_t>&))
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
    //printf("crcToCheck: %04X\n", msg[len - 1]);
    //printf("crcCalculated: %04X\n", msg[len - 1]);
    //printf("sendAddress: %04X\n", msg[0]);
    //printf("sendMsgLen: %04X\n", msg[1]);

    if (len > 3 && msg[2] == 0)
    { // check if single package
        msg.erase(msg.begin(),msg.begin()+3);
        //printResponse(msg);
        action(msg);
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
            action(telegramBuffer);
            //printResponse(telegramBuffer);//do something with telegramBuffer;

        }
    }

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


string buildTelegramRS232(vector<__uint16_t> data){ //TODO finish
    WORD len= data.size() + 1;
    data.insert(data.begin(),len);
    data.insert(data.begin(),0x1410);

    data.push_back( vecCrc(data));
    std::string telegram;
    telegram.append({0x2});
    printf("Msg to Send: ");
    for (const auto& i: data)
    {
        telegram.append(wordToString(i));
        printf("%04X,",i);
    }
    telegram.append({0x3});
    printf("\n");
    return telegram;
}

WORD vecCrc(vector<WORD> data){

    WORD crc = 0xFFFF;

for(const auto& d : data )
	{
	crc = ( (crc << 8) | ((BYTE)( d >> 8 ) ) ) ^ crctab[crc>>8];
	crc = ( (crc << 8) | ((BYTE) d )) ^ crctab[crc>>8];
	}
return crc;
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

// TODO clean up
// calculates an array of words from a given msg_string, returns the length of res
// int parseMsgRS232(msg_String, length of msg, pointer to result, idx of result)
int convertMsgToWords(vector<char> &msg,  vector<__uint16_t> &res)
{
    vector<__uint16_t> tempRes;
    int len= msg.size();
    res.clear();
    // printf("msg:%s\n",msg);

    for (int a = 1; a < (len - 2); a += 2)
    {
        char hexSubString[3] = {msg[a], msg[a + 1], '\0'};
        __uint16_t number = strtol(hexSubString, nullptr, 16);
        // printf("a:%i\tread: %s\tas: %lx\n",a,hexSubString,number);
        tempRes.push_back(number);
    }
    //printf("msg in words: ");//DEBUG

    for (int i = 0; i < (len - 2) / 2; i += 2)
    {
        __uint16_t tmp = (tempRes[i] << 8) + tempRes[i + 1];
        res.push_back(tmp);
        //printf("%04x,", tmp);//DEBUG
    }
    //printf("\n");//DEBUG
    //printf("msg as ascii: ");
    //for (int i = 0; i < (len - 2) / 2; i += 2)
    //{
    //    printf(" %c %c,",std::min(std::max(0x20,(int)tempRes[i]), 0x7E),std::min(std::max(0x20,(int)tempRes[i+1]), 0x7E) );
    //}
    //printf("\n");
    return (len - 2) / 4;
}

bool readBufferRS232(const char *readBuf, vector<char> &msgBuf, int bytesRead, bool continueLastMsg,
                     vector<__uint16_t> &telegramBuffer, void *action (vector<__uint16_t>&))
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

                    parseMsgRS232(result, telegramBuffer, action); // do something with result

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