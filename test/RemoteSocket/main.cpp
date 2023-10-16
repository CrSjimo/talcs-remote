#include <iostream>
#include <thread>
#include "RemoteSocket.h"

using namespace talcs;

RemoteSocket *rs;

class Listener : public RemoteSocket::Listener {
public:
    void socketStatusChanged(int newStatus, int oldStatus) override {
        std::cout << newStatus << std::endl;
    }
};

int main() {
    rs = new RemoteSocket(28081, 28082);
    rs->addListener(new Listener);
    std::cout << rs->startServer() << std::endl;
    std::cout << rs->startClient() << std::endl;
    std::cout << "started" << std::endl;
    for(;;);
    return 0;
}
