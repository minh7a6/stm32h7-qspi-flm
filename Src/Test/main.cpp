#include "Config.hpp"
#include <array>
#include "Board.hpp"
extern "C"
{
    int main();
}
__attribute__((section("external"))) int do_math(int a, int b)
{
    return a + b;
}

int main()
{
    SystemInit();
    SCB_InvalidateICache();
    SCB_InvalidateDCache();
    SCB_EnableICache();
    SCB_EnableDCache();
    qspi_driver drv(QUADSPI);
    FLASH_CLASS flash(drv);
    drv.deinit();
    Board::gpio_deinit();
    Board::gpio_init();
    drv.init(qspi_init);
    int res = flash.init();
    if (res != 0)
    {
        while (1)
            ;
    }
    if (flash.mmap() != 0)
    {
        while (1)
            ;
    }
    while (1)
    {
        __NOP();
    }
}