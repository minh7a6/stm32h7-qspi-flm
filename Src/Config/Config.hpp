#ifndef __CONFIG_HPP
#define __CONFIG_HPP

#include "w25qxjv.hpp"
#include "Board.hpp"
#if defined(W25Q64JV) || defined(W25Q32JV) || defined(W25Q16JV)

#if defined(W25Q64JV)
using FLASH_CLASS = w25q64jv;
#endif
#if defined(W25Q32JV)
using FLASH_CLASS = w25q32jv;
#endif
#if defined(W25Q16JV)
using FLASH_CLASS = w25q16jv;
#endif
// AHB3 is 200MHz max, with 400MHz, it is divided by 2
constexpr auto presc = qspi_driver::get_presc(Board::get_clk() / 2, FLASH_CLASS::get_max_clk());
constexpr uint32_t flash_size = FLASH_CLASS::get_size();
constexpr uint32_t sector_size = FLASH_CLASS::get_sect_size();
constexpr uint32_t pg_size = FLASH_CLASS::get_pg();
constexpr auto fsize = qspi_driver::get_fsize(flash_size);
constexpr qspi_driver::init_t qspi_init = {
    presc, // prescaler
    4,     // threshold
    fsize, // fsize
    0,     // chip sel high time
    false, // ckmode low
    false, // sample shift
};
#endif

#endif