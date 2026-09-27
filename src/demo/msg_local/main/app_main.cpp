#include <cstdio>
#include <iostream>

int main();

extern "C" void app_main()
{
    const int result = main();

    std::cout.flush();
    fflush(stdout);

    printf("\n[ESP] msg_test returned %d\n", result);
}
