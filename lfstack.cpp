#ifndef STREAMING
#include "lfstack_basic.h"
#else
#include "lfstack_streaming.h"
#endif

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <tuple>
#include <thread>
#include <ctime>

void DoWork(LockFreeStack *stack)
{
    for(;;)
    {
        if(std::rand() % 2)
        {
            uint32_t val = static_cast<uint32_t>(std::rand());
            stack->Push(val);
        }
        else
        {
            std::ignore = stack->Pop();
        }
    }
}

int main()
{
    std::srand(std::time(0));
    LockFreeStack stack;
    for(int i = 0; i < 100; i++)
    {
        std::thread t(DoWork, &stack);
        t.detach();
    }

    for(;;)
    {
        uint64_t old_count = stack.GetOperationCount();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        uint64_t new_count = stack.GetOperationCount();

        uint64_t ops_per_second = new_count - old_count;
        std::printf("%llu ops per second\n", ops_per_second);
    }
    return 0;
}
