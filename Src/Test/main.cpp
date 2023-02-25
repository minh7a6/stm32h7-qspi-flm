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
__attribute__((aligned(32)))
int a = 0;
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
    // watchdog::refresh();
    if (flash.mmap() != 0)
    {
        while (1)
            ;
    }

    SCB_InvalidateDCache_by_Addr(&a, sizeof(a));
    a = do_math(6, 3);
    __DSB();
    while (a != (6 + 3))
        ;
    while (1)
    {
        __NOP();
    }
}