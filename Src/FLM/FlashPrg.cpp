/* Flash OS Routines
 * Copyright (c) 2009-2015 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/** @file FlashPrg.c */

#include "FlashOS.hpp"
#include "FlashPrg.hpp"
#include "Config.hpp"
#include "Board.hpp"
#include "watchdog.hpp"
constexpr uint32_t flashOK = 0;
constexpr uint32_t flashFail = 1;
constexpr uint32_t timeoutMS = 6000;
extern "C"
{
    void SystemInit(void);
    uint32_t Init(uint32_t adr, uint32_t clk, uint32_t fnc)
    {
        // Called to configure the SoC. Should enable clocks
        //  watchdogs, peripherals and anything else needed to
        //  access or program memory. Fnc parameter has meaning
        //  but currently isnt used in MSC programming routines
        (void)adr;
        (void)clk;
        (void)fnc;
        SystemInit();
#ifdef CORE_CM7
        SCB_EnableICache();
        SCB_EnableDCache();
#endif
        qspi_driver drv(QUADSPI);
        // get drv ref
        w25q64jv flash(drv);
        drv.deinit();
        Board::gpio_deinit();
        Board::gpio_init();
        drv.init(qspi_init);
        constexpr auto wdg_init = watchdog::get_init(timeoutMS);
        static_assert(wdg_init.has_value());
        watchdog::init(wdg_init.value());
        watchdog::refresh();
        int res = flash.init();
        if (res != 0)
        {
            return flashFail;
        }
        return flashOK;
    }

    uint32_t UnInit(uint32_t fnc)
    {
        // When a session is complete this is called to powerdown
        //  communication channels and clocks that were enabled
        //  Fnc parameter has meaning but isnt used in MSC program
        //  routines
        (void)fnc;
        qspi_driver drv(QUADSPI);
        // get drv ref
        drv.deinit();
        Board::gpio_deinit();
        return flashOK;
    }

    uint32_t BlankCheck(uint32_t adr, uint32_t sz, uint8_t pat)
    {
        // Check that the memory at address adr for length sz is
        // empty or the same as pat
        qspi_driver drv(QUADSPI);
        // get drv ref
        w25q64jv flash(drv);
        watchdog::refresh();
        if (flash.blank_check(reinterpret_cast<void *>(adr), sz, pat) != 0)
        {
            return flashFail;
        }
        return flashOK;
    }

    uint32_t EraseChip(void)
    {
        // Execute a sequence that erases the entire of flash memory region
        int res;
        qspi_driver drv(QUADSPI);
        // get drv ref
        w25q64jv flash(drv);
        watchdog::refresh();
        res = flash.erase_chip();
        if (res != 0)
        {
            return flashFail;
        }
        return flashOK;
    }

    uint32_t EraseSector(uint32_t adr)
    {
        // Execute a sequence that erases the sector that adr resides in
        int res;
        qspi_driver drv(QUADSPI);
        // get drv ref
        w25q64jv flash(drv);
        watchdog::refresh();
        res = flash.erase_sector(reinterpret_cast<void *>(adr));
        if (res != 0)
        {
            return flashFail;
        }
        return flashOK;
    }

    uint32_t ProgramPage(uint32_t adr, uint32_t sz, uint32_t *buf)
    {
        // Program the contents of buf starting at adr for length of sz
        qspi_driver drv(QUADSPI);
        FLASH_CLASS flash(drv);
        const auto destAddr = reinterpret_cast<void *>(adr);
        watchdog::refresh();
        if (flash.program_page(destAddr, sz * sizeof(*buf), buf) != 0)
        {
            return flashFail;
        }
        return flashOK;
    }

    uint32_t Verify(uint32_t adr, uint32_t sz, uint32_t *buf)
    {
        // Given an adr and sz compare this against the content of buf
        qspi_driver drv(QUADSPI);
        FLASH_CLASS flash(drv);
        const auto destAddr = reinterpret_cast<void *>(adr);
        watchdog::refresh();
        if (flash.verify(destAddr, sz * sizeof(*buf), buf) != 0)
        {
            return flashFail;
        }
        return flashOK;
    }
}