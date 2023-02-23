#ifndef W25Q64JV_HPP
#define W25Q64JV_HPP

#include "QspiFlash.hpp"

class w25q64jv final : public QspiFlash
{
public:
    w25q64jv(qspi_driver &drv) : QspiFlash(drv) {}
    int init();
    int program_page(void *dest, const uint32_t size, void *src);
    int verify(void *dest, const uint32_t size, void *src);
    int blank_check(void *dest, const uint32_t size, uint8_t data);
    int erase_sector(void *adr);
    int erase_chip();
    int read(void *dest, uint32_t size, void *buff);
    int mmap();
    static constexpr uint32_t get_size() { return size; }
    static constexpr uint32_t get_pg() { return pg_size; }
    static constexpr uint32_t get_sect_size() { return sector_size; }

private:
    int enable_qio();
    int restart_chip();
    int wen();
    int poll_busy();
    static constexpr uint32_t size = 0x800000;
    static constexpr uint32_t pg_size = 0x100;
    static constexpr uint32_t sector_size = 0x00010000;
    static constexpr uint8_t alternate_byte = 0xf0;
    enum cmd
    {
        write_enable = 0x06,
        read_status_reg = 0x05,
        write_vol_cfg_reg = 0x31,
        sector_erase = 0xd8,
        chip_erase = 0xc7,
        quad_in_fast_prog = 0x32,
        read_conf_reg = 0x35,
        quad_out_fast_read = 0xeb,
        reset_enable = 0x66,
        reset_execute = 0x99,
    };
};

#endif