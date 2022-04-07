// Tutorial found at: https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/


#include "comtest.h"
//#include "RS232Comm.h"
#include "MessageHandlerEnternet.h"
//#include "MessageHandler.h"

#include "MessagePrinter.h"
#include "AutoMeasure.h"

#define PORT 49152
#define ethBuffer 1024

void autoProgram(int sock){
    auto sendFunc= [sock](std::vector<WORD> &pData){
        std::vector<__uint8_t> dataToSend;
        dataToSend= msgHE::buildTelegram(pData);
        send(sock , dataToSend.data() , dataToSend.size() , 0 );
    };

    auto readFunc=[sock](void actionFunc(std::vector<WORD>&)){
        std::function<void(std::vector<WORD>&)> action=[actionFunc](std::vector<WORD>& msg){actionFunc(msg);};
        unsigned char buffer[ethBuffer] = {0};
        unsigned int valRead;
        std::vector<__uint8_t> msgBuff;
        bool readAgain=true;
        while (readAgain){
            size_t wait=0;
            while (wait ==0) {
                wait = recv(sock, buffer, ethBuffer, MSG_PEEK);
                printf("peek: %i",wait);
            }
            printf("start reading\n");
            valRead = (int)read( sock , buffer, ethBuffer);
            printf("done reading\n");
            readAgain = msgHE::readBuffer(buffer,msgBuff,valRead,action);
        }
    };
    //performAutoMeasure(sendFunc,readFunc);
    AutoMeasure::performAutoMeasure(sendFunc,readFunc);
}


int inputMsgEther(asio::ip::tcp::socket& socket, asio::error_code& ec){ //TODO add auto split msg
    std::string inputString;
    int repeat=1;
    std::vector<__uint8_t> dataToSend;
    std::regex wordRegex("^([A-Fa-f0-9]{4},)*([A-Fa-f0-9]{4},?$)");
    std::cout << "enter Command: ";
    std::cin >> inputString;

    if (inputString == "q" || inputString == "Q")
        repeat=0;
    else {
        if(regex_match(inputString, wordRegex)){
            inputString.erase(remove(inputString.begin(),inputString.end(),','),inputString.end());
            inputString.insert(0,"q");
            inputString.append(" ");

            std::vector<__uint16_t> res;

            std::vector<char> charInput(inputString.begin(),inputString.end());
            msgHE::convertMsgToWords(charInput,res);
            dataToSend= msgHE::buildTelegram(res);
            printf("res: ");
            for(auto& r:res){
                printf("%04X,",r);
            }
            printf("\n");
            printf("to send: ");
            for(auto& r:dataToSend){
                printf("%02X,",r);
            }
            printf("\n");
            
            socket.write_some(asio::buffer(dataToSend),ec);

        }else{
            std::cout << "invalid Command" << std::endl;
            repeat= -1;
        }

    }
    return repeat;
}



int main()
{
    asio::error_code ec;
    asio::io_context context;
    asio::ip::tcp::endpoint endpoint(asio::ip::make_address("192.168.1.10",ec),49152);
    asio::ip::tcp::socket socket(context);
    //int sock = 0, valRead;
    //struct sockaddr_in serv_addr;
    std::vector<WORD>msg={0x0202,0x0010};
    std::vector<__uint8_t> dataToSend;
    std::vector<__uint8_t> msgBuff;
    //std::function<void(std::vector<WORD>&)> printFun= [](std::vector<WORD>& msg){ printResponse(msg);};



    socket.connect(endpoint, ec);

    //unsigned char hello[] = {0x02,0x55,0x53,0x50,0x00,0x00,0x00,0x02,0x01,0x02,0x03};
    
    

    
    while(inputMsgEther(socket, ec));
    /*dataToSend= buildTelegram(msg);
    send(sock , dataToSend.data() , dataToSend.size() , 0 );
    printf("Hello message sent\n");

    bool readAgain=true;
    while (readAgain){
        valRead = (int)read( sock , buffer, ethBuffer);
        readAgain = readBuffer(buffer,msgBuff,valRead,printFun);
    }*/
    //autoProgram(sock);



    socket.close();

    return 0;
}