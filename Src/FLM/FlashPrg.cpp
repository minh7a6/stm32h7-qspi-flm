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
constexpr uint32_t timeoutMS = 25000;

#define SUPPORT_NATIVE_VERIFY (0)        // Non-memory mapped flashes only. Flash cannot be read memory-mapped
#define SUPPORT_NATIVE_READ_FUNCTION (0) // Non-memory mapped flashes only. Flash cannot be read memory-mapped
#define SUPPORT_ERASE_CHIP (1)           // To potentially speed up production programming: Erases whole flash bank / chip with special command
#define SUPPORT_TURBO_MODE (0)           // Currently available for Cortex-M only
#define SUPPORT_SEGGER_OPEN_ERASE (1)    // Flashes with uniform sectors only. Speed up erase because 1 OFL call may erase multiple sectors

/*********************************************************************
 *
 *       Defines (fixed)
 *
 **********************************************************************
 */

// Smallest amount of data that can be programmed. <PageSize> = 2 ^ Shift. Shift = 3 => <PageSize> = 2^3 = 8 bytes
static constexpr uint32_t PAGE_SIZE_SHIFT = qspi_driver::get_fsize(w25q64jv::get_pg()) + 1;          

// Flashes with uniform sectors only. <SectorSize> = 2 ^ Shift. Shift = 12 => <SectorSize> = 2 ^ 12 = 4096 bytes
static constexpr uint32_t SECTOR_SIZE_SHIFT = qspi_driver::get_fsize(w25q64jv::get_sect_size()) + 1; 

static_assert(PAGE_SIZE_SHIFT > 0);
static_assert(SECTOR_SIZE_SHIFT > 0);
//
// Default definitions for optional functions if not compiled in
// Makes Api table code further down less ugly
//
#if (SUPPORT_ERASE_CHIP == 0)
#define EraseChip nullptr
#endif
#if (SUPPORT_NATIVE_VERIFY == 0)
#define Verify nullptr
#endif
#if (SUPPORT_NATIVE_READ_FUNCTION == 0)
#define SEGGER_OPEN_Read nullptr
#endif
#if (SUPPORT_SEGGER_OPEN_ERASE == 0)
#define SEGGER_OPEN_Erase nullptr
#endif
#if (SUPPORT_TURBO_MODE == 0)
#define SEGGER_OPEN_Start NULL
#endif
extern "C"
{
  const SEGGER_OFL_API SEGGER_OFL_Api __attribute__((section("PrgCode"))) = { // Mark start of <PrgCode> segment. Non-static to make sure linker can keep this symbol.
      NULL,
      Init,
      UnInit,
      EraseSector,
      ProgramPage,
      BlankCheck,
      EraseChip,
      Verify,
      NULL,
      SEGGER_OPEN_Read,
      SEGGER_OPEN_Program,
      SEGGER_OPEN_Erase,
      SEGGER_OPEN_Start};
}

/* Quirks: having dummy data here to not optimize out the PrgData NOBITS*/
// volatile uint32_t dummydata __attribute__((section("PrgDataBss")));
void SystemInit(void);
int Init(uint32_t adr, uint32_t clk, uint32_t fnc)
{
  // Called to configure the SoC. Should enable clocks
  //  watchdogs, peripherals and anything else needed to
  //  access or program memory. Fnc parameter has meaning
  //  but currently isnt used in MSC programming routines
  (void)adr;
  (void)clk;
  (void)fnc;
  // *(uint32_t *)0xe000edf0=0xa05f0000; // enable irq in debug
  SystemInit();
  SCB_InvalidateICache();
  SCB_InvalidateDCache();
  SCB_EnableICache();
  SCB_EnableDCache();
  qspi_driver drv(QUADSPI);
  // get drv ref
  FLASH_CLASS flash(drv);
  // Board::rcc_config();
  drv.deinit();
  Board::gpio_deinit();
  Board::gpio_init();
  drv.init(qspi_init);
  int res = flash.init();
  if (res != 0)
  {
    return flashFail;
  }
  // constexpr auto wdg_init = watchdog::get_init(timeoutMS);
  // static_assert(wdg_init.has_value());
  // watchdog::init(wdg_init.value());
  // watchdog::refresh();
  if (flash.mmap() != 0)
  {
    return flashFail;
  }
  /* Quirks: Trigger Read access, otherwise abort will stuck */
  uint32_t a = *(uint32_t *)(QSPI_BASE);
  a++;
  return flashOK;
}

int UnInit(uint32_t fnc)
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

int BlankCheck(uint32_t adr, uint32_t sz, uint8_t pat)
{
  // Check that the memory at address adr for length sz is
  // empty or the same as pat
  qspi_driver drv(QUADSPI);
  FLASH_CLASS flash(drv);
  adr -= QSPI_BASE;
  if (flash.blank_check(reinterpret_cast<void *>(adr), sz, pat) != 0)
  {
    return flashFail;
  }
  return flashOK;
}
#if SUPPORT_ERASE_CHIP
int EraseChip(void)
{
  // Execute a sequence that erases the entire of flash memory region
  int res;
  qspi_driver drv(QUADSPI);
  FLASH_CLASS flash(drv);
  res = flash.erase_chip();
  if (res != 0)
  {
    return flashFail;
  }
  return flashOK;
}
#endif
int EraseSector(uint32_t adr)
{
  // Execute a sequence that erases the sector that adr resides in
  int res;
  qspi_driver drv(QUADSPI);
  adr -= QSPI_BASE;
  FLASH_CLASS flash(drv);
  res = flash.erase_sector(reinterpret_cast<void *>(adr));
  if (res != 0)
  {
    return flashFail;
  }
  return flashOK;
}

int ProgramPage(uint32_t adr, uint32_t sz, uint8_t *buf)
{
  // Program the contents of buf starting at adr for length of sz
  qspi_driver drv(QUADSPI);
  FLASH_CLASS flash(drv);
  adr -= QSPI_BASE;
  const auto destAddr = reinterpret_cast<void *>(adr);
  if (flash.program_page(destAddr, sz * sizeof(*buf), buf) != 0)
  {
    return flashFail;
  }
  return flashOK;
}
#if SUPPORT_NATIVE_VERIFY
uint32_t Verify(uint32_t adr, uint32_t sz, uint8_t *buf)
{
  // Given an adr and sz compare this against the content of buf
  qspi_driver drv(QUADSPI);
  FLASH_CLASS flash(drv);
  adr -= QSPI_BASE;
  const auto destAddr = reinterpret_cast<void *>(adr);
  watchdog::refresh();
  if (flash.verify(destAddr, sz * sizeof(*buf), buf) != 0)
  {
    return flashFail;
  }
  return flashOK;
}
#endif
int SEGGER_OPEN_Program(uint32_t DestAddr, uint32_t NumBytes, uint8_t *pSrcBuff)
{
  uint32_t NumPages;
  int r;
  qspi_driver drv(QUADSPI);
  FLASH_CLASS flash(drv);
  drv.deinit();
  Board::gpio_deinit();
  Board::gpio_init();
  drv.init(qspi_init);
  int res = flash.init();
  if (res != 0)
  {
    return flashFail;
  }
  NumPages = (NumBytes >> PAGE_SIZE_SHIFT);
  r = 0;
  do
  {
    r = ProgramPage(DestAddr, (1uL << PAGE_SIZE_SHIFT), (pSrcBuff));
    if (r < 0)
    {
      return r;
    }
    DestAddr += (1uL << PAGE_SIZE_SHIFT);
    pSrcBuff += (1uL << PAGE_SIZE_SHIFT);
  } while (--NumPages);
  return r;
}

#if SUPPORT_SEGGER_OPEN_ERASE
int SEGGER_OPEN_Erase(uint32_t SectorAddr, uint32_t SectorIndex, uint32_t NumSectors)
{
  int r;
  qspi_driver drv(QUADSPI);
  FLASH_CLASS flash(drv);
  drv.deinit();
  Board::gpio_deinit();
  Board::gpio_init();
  drv.init(qspi_init);
  int res = flash.init();
  if (res != 0)
  {
    return flashFail;
  }
  (void)SectorIndex;
  //   _FeedWatchdog();
  r = 0;
  do
  {
    r = EraseSector(SectorAddr);
    if (r)
    {
      break;
    }
    SectorAddr += (1 << SECTOR_SIZE_SHIFT);
  } while (--NumSectors);
  return r;
}
#endif

#if SUPPORT_NATIVE_READ_FUNCTION
int SEGGER_OPEN_Read(uint32_t Addr, uint32_t NumBytes, uint8_t *pDestBuff)
{
  //
  // Read function
  // Add your code here...
  //
  //_FeedWatchdog();
  Addr -= QSPI_BASE;

  qspi_driver drv(QUADSPI);
  FLASH_CLASS flash(drv);
  drv.deinit();
  Board::gpio_deinit();
  Board::gpio_init();
  drv.init(qspi_init);
  int res = flash.init();
  if (res != 0)
  {
    return flashFail;
  }
  if (flash.read(reinterpret_cast<void *>(Addr), NumBytes, pDestBuff) != 0)
  {
    return 0;
  }
  return NumBytes;
}
#endif