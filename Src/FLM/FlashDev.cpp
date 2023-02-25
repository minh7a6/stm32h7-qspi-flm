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

/** @file FlashDev.c */

#include "FlashOS.hpp"
#include "Config.hpp"
#define FLASH_DRV_VERS (0x0100 + VERS) // Driver Version, do not modify!

extern "C"
{
    struct FlashDevice const FlashDevice __attribute__((section("DevDscr"))) = {
        FLASH_DRV_VERS,             // Driver Version, do not modify!
        FLASH_LDR_NAME,             // Device Name (128 chars max)
        EXTSPI,                     // Device Type
        QSPI_BASE,                  // Base Address                                                                                                                                                                                                                                                                                                                                                      ,                 // Device Start Address
        flash_size,                 // Device Size
        pg_size,                    // Programming Page Size
        0x00000000,                 // Reserved, must be 0
        0xFF,                       // Initial Content of Erased Memory
        100,                        // Program Page Timeout 100 mSec
        3000,                       // Erase Sector Timeout 3000 mSec
        {{sector_size, 0x00000000}, // Sector Size {1kB, starting at address 0}
         {SECTOR_END}}};
}