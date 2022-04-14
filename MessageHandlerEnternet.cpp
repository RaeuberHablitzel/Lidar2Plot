//
// Created by eric on 01.04.22.
//

#include "MessageHandlerEnternet.h"

namespace msgHE{


    const std::array<__uint8_t,4> startSeq={0x02,0x55,0x53,0x50};


    bool readBuffer(std::vector<__uint8_t>& readBuf, std::vector<unsigned char> &msgBuf, std::function<void(std::vector<__uint16_t>&)> &action){
    __uint32_t msgLen = 0;
        int i =0;
        unsigned int msgSize=0;
        bool msgLenKnown=false;

        for (i=0; i<readBuf.size();++i){
            msgSize=msgBuf.size();
            //printf("%02X,",readBuf[i]);
            if (msgSize<4){ //look for startSequence
                if (readBuf[i]==startSeq[msgSize]){ // check startSequence
                    msgBuf.push_back(readBuf[i]);
                    //printf("header found\n");
                } else{ // no start
                msgBuf.clear();
                }
            } else if(msgSize<8){ // get msg len
                msgBuf.push_back(readBuf[i]);
                //printf("len\n");
            }else {
                if (!msgLenKnown) {
                    msgLen = msgBuf[4]<<24 | msgBuf[5]<<16 | msgBuf[6]<<8 | msgBuf[7];
                    msgLenKnown=true;
                    //printf("msgLen:%i\n",msgLen);
                }
                if (msgSize < msgLen+8){ // get msg payload
                    msgBuf.push_back(readBuf[i]);
                    //printf("Data\n");
                }else{
                    __uint8_t cs=0x00;
                    for (int j = 8; j<msgLen+8; ++j){
                        cs^=msgBuf[j];
                    }
                    //printf("cs: %02X\ncheck: %i\n",cs,cs==readBuf[i]);
                    if (cs==readBuf[i]){
                        std::vector<__uint16_t> msg;
                        for (int j = 8; j<msgLen+8; j+=2){
                            msg.push_back((msgBuf[j]<<8)|msgBuf[j+1]);
                        }
                        //printf("call action\n");
                        action(msg);
                        msgBuf.clear();
                    }else{ //checksum error
                        msgBuf.clear();

                    }
                }
            }

        }
        return !msgBuf.empty();
    }

    std::vector<__uint8_t> buildTelegram(std::vector<WORD> &msg){
        std::vector<__uint8_t> res = {0x02,0x55,0x53,0x50};
        __uint32_t len= msg.size()*2;
        for (int i=3; i>=0; --i){
            res.push_back((len>>(i*8))&0xFF);
        }
        for(const auto& m : msg ){
            res.push_back((m>>8)&0xFF);
            res.push_back(m&0xFF);
        }
        __uint8_t cs=0x00;
        for(const auto& m : msg )
        {
            cs^=(m>>(8))&0xFF;
            cs^=m&0xFF;
        }
        res.push_back(cs);
        return res;
    }

    // TODO clean up
    // calculates an array of words from a given msg_string, returns the length of res
    // int parseMsgRS232(msg_String, length of msg, pointer to result, idx of result)
    int convertMsgToWords(std::vector<char> &msg,  std::vector<__uint16_t> &res)
    {
        std::vector<__uint16_t> tempRes;
        unsigned int len= msg.size();
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
        return ((int)len - 2) / 4;
    }
}