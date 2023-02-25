#ifndef FLASH_NOR_HPP
#define FLASH_NOR_HPP

#include "qspi.hpp"
class QspiFlash
{
protected:
    qspi_driver &_drv;

public:
    virtual int program_page(void *dest, uint32_t size, void *src) { return 1; }
    virtual uint32_t verify(void *dest, uint32_t size, void *src) { return 0; }
    virtual int blank_check(void *dest, uint32_t size, uint8_t data) { return 1; }
    virtual int erase_sector(void *adr) { return 1; }
    virtual int erase_chip() { return 1; }
    virtual int read(void *dest, uint32_t size, void *buff) { return 1; }
    int abort() { return _drv.abort(); }
    virtual int mmap() { return 1; }
    QspiFlash(qspi_driver &drv) : _drv(drv) {}
};

#endif