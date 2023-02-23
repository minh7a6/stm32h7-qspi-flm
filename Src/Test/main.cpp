#include "Config.hpp"
#include <array>
#include "Board.hpp"

extern "C"
{
    int main()
    {
        SCB_EnableICache();
        SCB_EnableDCache();
        Board::gpio_init();
        qspi_driver drv(QUADSPI);
        // get drv ref
        FLASH_CLASS flash(drv);
        drv.init(qspi_init);
        flash.init();
        std::array<uint8_t, 256> wr_buff;
        // Pad data with 0 .. size
        for (std::size_t i = 0; i < wr_buff.size(); i++)
        {
            wr_buff[i] = i;
        }
        uint8_t *addr = reinterpret_cast<uint8_t *>(3 * flash.get_sect_size());
        // Erase sector
        auto res1 = flash.erase_sector(addr);
        if (res1 != 0)
        {
            while (1)
                ;
        }
        // Check for blank
        auto res2 = flash.blank_check(addr, wr_buff.size(), 0xff);
        if (res2 != 0)
        {
            while (1)
                ;
        }
        // Program the page
        auto res3 = flash.program_page(addr, wr_buff.size(), wr_buff.data());
        if (res3 != 0)
        {
            while (1)
                ;
        }
        // Verify
        auto res4 = flash.mmap();
        if (res4 != 0)
        {
            while (1)
                ;
        }
        addr = reinterpret_cast<uint8_t *>(3 * flash.get_sect_size() + QSPI_BASE);
        for (std::size_t i = 0; i < wr_buff.size(); i++)
        {
            if (addr[i] != wr_buff[i])
            {
                while (1)
                    ;
            }
        }
        while (true)
        {

            __NOP();
        }
        return 0;
    }
}
