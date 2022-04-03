// Tutorial found at: https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/

#include "comtest.h"
#include "MessageHandler.h"
#include "MessagePrinter.h"
#include "AutoMeasure.h"
#include "MessageHandlerEnternet.h"

using namespace std;

void sendCommand(int serial_port, vector<WORD> data)
{

    string telegram = buildTelegramRS232(data);
    cout << telegram << endl;
    write(serial_port, telegram.c_str(), telegram.size());

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
            convertMsgToWords(charInput,res);
            sendCommand(SerialPort,res);
        }else{
            cout << "invalid Command" << endl;
           repeat= -1;
        }

    }
    return repeat;
}

int main()
{
    //rs232 stuff
    /*int serial_port = open(com_port, O_RDWR);
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

    /*
    //Auto Mode
    auto sendFunc= [serial_port](vector<WORD> &pData){ sendCommand(serial_port,pData);};
    auto readFunc=[serial_port](void actionFunc(vector<WORD>&)){
        int bytes_available=0;
        char read_buf[buf_size];
        bool incompleteMsg = false;
        vector<__uint16_t> telegramBuffer;
        vector<char> msg_buf;
        while (bytes_available == 0) {
                ioctl(serial_port, FIONREAD, &bytes_available);
            }
            do {
                memset(&read_buf, '\0', sizeof(read_buf)); // maybe not needed

                int num_bytes = read(serial_port, &read_buf, sizeof(read_buf));

                // n is the number of bytes read. n may be 0 if no bytes were received, and can also be -1 to signal an error.
                if (num_bytes < 0) {
                    printf("Error reading: %s", strerror(errno));
                    return;
                }
                incompleteMsg = readBufferRS232(read_buf, msg_buf, num_bytes, incompleteMsg, telegramBuffer,actionFunc);
                ioctl(serial_port, FIONREAD, &bytes_available);
            } while (incompleteMsg || bytes_available > 0);};
    //performAutoMeasure(sendFunc,readFunc);
    AutoMeasure::performAutoMeasure(sendFunc,readFunc);
   //Manual Mode
   */
    /*
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





                incompleteMsg = readBufferRS232(read_buf, msg_buf, num_bytes, incompleteMsg, telegramBuffer,
                                                printResponse);
                ioctl(serial_port, FIONREAD, &bytes_available);
            } while (incompleteMsg || bytes_available > 0);
        }
    }
    printf("returned: %i\n", incompleteMsg);



    close(serial_port);*/

    std::function<void (std::vector<WORD>&)> action=[](std::vector<WORD>&msg){
        printf("received:");
        for (const auto& r : msg){
        printf("%04X,",r);
        }
    printf("\n");
    };

    unsigned char testMsg[15]={0x02,0x55,0x53,0x50,0x00,0x00,0x00,0x06,0x81,0x02,0x00,0x00,0x00,0x01,0x82};
    char test[15];
    for (int i=0;i<15;++i){
        test[i]=testMsg[i];
    }
    std::vector<char> msgBuf;
    std::vector<WORD>msg={0x08102,0x0000,0x0001};
    std::vector<__uint8_t> res = buildTelegram(msg);
    for (const auto& r : res){
        printf("%02X,",r);
    }
    printf("\n");
    readBuffer( test, msgBuf, 15, action);
    return 0;
}