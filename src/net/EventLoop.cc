#include "EventLoop.h"
#include <iostream>

using namespace inet;

int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        std::cout << "eventfd error: " << errno << std::endl;
    }

    return evtfd;
}

