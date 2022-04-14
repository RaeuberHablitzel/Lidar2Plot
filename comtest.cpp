// Tutorial found at: https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/


#include <thread>
#include <chrono>
#include "comtest.h"
//#include "RS232Comm.h"
#include "MessageHandlerEnternet.h"
//#include "MessageHandler.h"

#include "MessagePrinter.h"
#include "AutoMeasure.h"

#define PORT 49152
#define ethBuffer 1024

void manualCom(asio::ip::tcp::socket &socket, asio::error_code &ec);

std::vector<__uint8_t> read_buff(30*1024);
std::vector<__uint8_t> msg_buff;





void grabSomeDate(asio::ip::tcp::socket& socket,std::function<void(std::vector<WORD> &)>&action){
    socket.async_read_some(asio::buffer(read_buff),[&](std::error_code ec, std::size_t len){
       if (!ec){
           std::vector<__uint8_t> resizedReadBuff(read_buff.begin(),read_buff.begin()+len);
           msgHE::readBuffer(resizedReadBuff,msg_buff,action);
           grabSomeDate(socket,action);
       }
    });

}





void autoProgram(asio::ip::tcp::socket& socket, asio::error_code& ec){
    auto sendFunc= [&socket,&ec](std::vector<WORD> &pData){
        std::vector<__uint8_t> dataToSend;
        dataToSend= msgHE::buildTelegram(pData);
        if (socket.is_open()){

            socket.write_some(asio::buffer(dataToSend),ec);

        }
        else{
            std::cout << "connection lost" << std::endl;
        }
    };

    auto readFunc=[&socket, &ec](void actionFunc(std::vector<WORD>&)){
        std::function<void(std::vector<WORD>&)> action=[actionFunc](std::vector<WORD>& msg){actionFunc(msg);};

        std::vector<__uint8_t> msgBuff;
        bool readAgain=true;
        while (readAgain){
            if (socket.is_open()) {
                size_t bytes = socket.available();

                while (!bytes) {
                    bytes = socket.available();

                    //std::cout << "available: " << bytes <<std::endl;
                }
                std::vector<__uint8_t> vBuffer(bytes);
                socket.read_some(asio::buffer(vBuffer), ec);
                std::cout << "reading done " <<std::endl;
                readAgain = msgHE::readBuffer(vBuffer, msgBuff, action);
            }else{
                std::cout << "connection lost" << std::endl;
            }
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
    int mode;
    std::cout << "select mode (manual(0), auto(1)): ";
    std::cin >> mode;
    asio::error_code ec;
    asio::io_context context;
    asio::io_context::work idleWork(context);
    std::thread thrContext= std::thread([&](){context.run();std::cout << "done"<<std::endl;return;});

    asio::ip::tcp::endpoint endpoint(asio::ip::make_address("192.168.1.10",ec),PORT);
    asio::ip::tcp::socket socket(context);
    std::function<void(std::vector<WORD>&)> parseFun= [](std::vector<WORD>& msg){ printResponse(msg);};
    std::vector<WORD>msg={0x0202,0x0010};

    //unsigned char hello[] = {0x02,0x55,0x53,0x50,0x00,0x00,0x00,0x02,0x01,0x02,0x03};


    socket.connect(endpoint, ec);
    if (mode==1){
        std::cout << "enter auto mode"<< std::endl;
        parseFun=[&socket,&ec](std::vector<WORD>& msg){
            auto sendFunc= [&socket,&ec](std::vector<WORD> &pData){
                std::vector<__uint8_t> dataToSend;
                dataToSend= msgHE::buildTelegram(pData);
                if (socket.is_open()){

                    socket.write_some(asio::buffer(dataToSend),ec);

                }
                else{
                    std::cout << "connection lost" << std::endl;
                }
            };
            AutoMeasure::asyncAutoMeasure(sendFunc,msg);
        };
        std::vector<WORD> data={0x0302};
        std::vector<__uint8_t>dataToSend= msgHE::buildTelegram(data);
        if (socket.is_open()){

            socket.write_some(asio::buffer(dataToSend),ec);

        }
        else{
            std::cout << "connection lost" << std::endl;
        }

    }
    grabSomeDate(socket,parseFun);
    if (mode==1){
        std::vector<WORD> data={0x0102};
        std::vector<__uint8_t>dataToSend= msgHE::buildTelegram(data);
        if (socket.is_open()){

            socket.write_some(asio::buffer(dataToSend),ec);

        }
        else{
            std::cout << "connection lost" << std::endl;
        }
    }





    while (inputMsgEther(socket,ec));






    socket.close();
    std::cout << "disconnected" << std::endl;
    context.stop();
    thrContext.join();

    return 0;
}

void manualCom(asio::ip::tcp::socket &socket, asio::error_code &ec) {
    std::vector<__uint8_t> dataToSend;
    std::vector<__uint8_t> msgBuff;
    int repeat=1;
    bool readAgain;
    std::function<void(std::vector<WORD>&)> printFun= [](std::vector<WORD>& msg){ printResponse(msg);};

    if (!ec){
    std::cout << "connected" << std::endl;
    }else{
    std::cout << "failed to connect: " << ec.message() << std::endl;
    }

    while(repeat){
        if (socket.is_open()){
            repeat=inputMsgEther(socket, ec);
            if(repeat>0) {
                do {
                    size_t bytes = socket.available();
                    while (!bytes) {
                        bytes = socket.available();
                    }
                    std::vector<__uint8_t> vBuffer(bytes);
                    socket.read_some(asio::buffer(vBuffer), ec);
                    readAgain = msgHE::readBuffer(vBuffer, msgBuff, printFun);
                } while (readAgain);
            }
        }else{
            std::cout << "connection lost" << std::endl;
        }
    }
}
