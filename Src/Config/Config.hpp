#ifndef __CONFIG_HPP
#define __CONFIG_HPP


#if defined(W25Q64JV)
#include "w25q64jv.hpp"
#define FLASH_CLASS w25q64jv
constexpr uint32_t flash_size = w25q64jv::get_size();
constexpr uint32_t sector_size = w25q64jv::get_sect_size();
constexpr uint32_t pg_size = w25q64jv::get_pg();
constexpr auto fsize = qspi_driver::get_fsize(flash_size);
constexpr qspi_driver::init_t qspi_init = {
    0,      // prescaler
    4,      // threshold
    fsize,  // fsize
    0,      // chip sel high time
    false,  // ckmode low
    false,  // sample shift
};
#endif

#endif