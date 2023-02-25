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

enum operation
{
  ERASE = 1,
  PROGRAM,
  VERIFY
};
/* Quirks: having dummy data here to not optimize out the PrgData NOBITS*/
volatile uint32_t dummydata __attribute__((section("PrgDataBss")));
extern "C"
{
  // /* start address for the .bss section. defined in linker script */
  // extern uint32_t _sbss;
  // /* end address for the .bss section. defined in linker script */
  // extern uint32_t _ebss;
  void SystemInit(void);
  void __libc_init_array();
}
int Init(uint32_t adr, uint32_t clk, uint32_t fnc)
{
  // Called to configure the SoC. Should enable clocks
  //  watchdogs, peripherals and anything else needed to
  //  access or program memory. Fnc parameter has meaning
  //  but currently isnt used in MSC programming routines
  (void)adr;
  (void)clk;
  // (void)fnc;
  // *(uint32_t *)0xe000edf0=0xa05f0000; // enable irq in debug
  // __asm("    ldr     r0, =_sbss\n"
  //       "    ldr     r1, =_ebss\n"
  //       "    mov     r2, #0\n"
  //       "    .thumb_func\n"
  //       "zero_loop:\n"
  //       "        cmp     r0, r1\n"
  //       "        it      lt\n"
  //       "        strlt   r2, [r0], #4\n"
  //       "        blt     zero_loop");
  SystemInit();

  SCB_InvalidateICache();
  SCB_InvalidateDCache();
  SCB_EnableICache();
  SCB_EnableDCache();
  // __libc_init_array();
  __disable_irq();
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
  if (fnc != PROGRAM)
  {
    if (flash.mmap() != 0)
    {
      return flashFail;
    }
  }

  /* Quirks: Trigger Read access, otherwise abort will stuck */
  // uint32_t a = *(uint32_t *)(QSPI_BASE);
  // a++;
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
  __enable_irq();
  SCB_DisableICache();
  SCB_DisableDCache();
  return flashOK;
}

int EraseChip(void)
{
  // Execute a sequence that erases the entire of flash memory region
  int res;
  qspi_driver drv(QUADSPI);
  FLASH_CLASS flash(drv);
  drv.deinit();
  Board::gpio_deinit();
  Board::gpio_init();
  drv.init(qspi_init);
  res = flash.init();
  if (res != 0)
  {
    return flashFail;
  }
  drv.init(qspi_init);
  res = flash.erase_chip();
  if (res != 0)
  {
    return flashFail;
  }
  return flashOK;
}

int EraseSector(uint32_t adr)
{
  // Execute a sequence that erases the sector that adr resides in
  adr -= QSPI_BASE;
  int res;
  qspi_driver drv(QUADSPI);
  FLASH_CLASS flash(drv);
  drv.deinit();
  Board::gpio_deinit();
  Board::gpio_init();
  drv.init(qspi_init);
  res = flash.init();
  if (res != 0)
  {
    return flashFail;
  }
  res = flash.erase_sector(reinterpret_cast<void *>(adr));
  if (res != 0)
  {
    return flashFail;
  }
  if (flash.mmap() != 0)
  {
    return flashFail;
  }
  return flashOK;
}

int ProgramPage(uint32_t adr, uint32_t sz, uint8_t *buf)
{
  // Program the contents of buf starting at adr for length of sz
  adr -= QSPI_BASE;
  qspi_driver drv(QUADSPI);
  FLASH_CLASS flash(drv);
  // drv.deinit();
  // Board::gpio_deinit();
  // Board::gpio_init();
  // drv.init(qspi_init);
  // int res = flash.init();
  // if (res != 0)
  // {
  //   return flashFail;
  // }
  const auto destAddr = reinterpret_cast<void *>(adr);
  if (flash.program_page(destAddr, sz * sizeof(*buf), buf) != 0)
  {
    return flashFail;
  }
  return flashOK;
}

// int BlankCheck(uint32_t adr, uint32_t sz, uint8_t pat)
// {
//   // Check that the memory at address adr for length sz is
//   // empty or the same as pat
//   qspi_driver drv(QUADSPI);
//   FLASH_CLASS flash(drv);
//   adr -= QSPI_BASE;
//   if (flash.blank_check(reinterpret_cast<void *>(adr), sz, pat) != 0)
//   {
//     return flashFail;
//   }
//   return flashOK;
// }

// uint32_t Verify(uint32_t adr, uint32_t sz, uint8_t *buf)
// {
//   // Given an adr and sz compare this against the content of buf
//   qspi_driver drv(QUADSPI);
//   FLASH_CLASS flash(drv);
//   adr -= QSPI_BASE;
//   const auto destAddr = reinterpret_cast<void *>(adr);
//   return adr + flash.verify(destAddr, sz * sizeof(*buf), buf);
// }