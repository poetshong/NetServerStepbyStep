#include "../src/EventLoop.h"

#include <iostream>
using namespace std;

int main()
{
    EventLoop loop;

    loop.runEvery(1s, []() 
                    {
                        cout << "Run at 1s\n";
                    });

    loop.runEvery(2s, []()
                    {
                        cout << "Run every 2s\n";  
                    });
    loop.runAfter(4s, [&]()
                    {
                        cout << "Run after 4s\n";
                    });
    loop.runAfter(10s, [&]()
                    {
                        cout << "End after 10s\n";
                        loop.quit();
                    });

    loop.loop();
}