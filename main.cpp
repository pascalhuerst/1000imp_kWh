#include <iostream>
#include "gpio.h"

#include <queue>
#include <atomic>
#include <thread>

using namespace std;

std::vector<int> gpiosNrs = { 71, 70, 73, 72, 75, 74 };


void run(atomic<bool> *terminateRequest,GPIOSysFs *gpio)
{
    while (!terminateRequest->load()) {
        std::cout << "E: " << gpio->getName() << "  " << gpio->getValueBlocking() << std::endl;
    }
}


int main()
{
    std::atomic<bool> terminateRequest;

    terminateRequest.store(false);

    std::queue<GPIOSysFs*> sensors;

    for (unsigned int i = 0  ; i<gpiosNrs.size(); i++) {
        auto sensor = new GPIOSysFs("F" + std::to_string((i)), gpiosNrs.at(i));
        sensors.push(sensor);

        new std::thread(run, &terminateRequest, sensor);
    }

   return 0;
}
