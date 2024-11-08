#include "ThreadPool.h"

#include <iostream>
#include <unistd.h>

using namespace inet;

void task()
{
    while (1)
    {
        std::cout << "test" << std::endl;
        sleep(1);
    }

}
int main()
{
    ThreadPool pool;
    pool.add(task);

    pool.start(10);

    while (1)
    {
        sleep(1);
    }
    

    return 0;
}
