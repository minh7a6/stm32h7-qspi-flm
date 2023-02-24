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

/** @file FlashPrg.h */

#ifndef FLASHPRG_H
#define FLASHPRG_H

#include "stdint.h"

#ifdef __cplusplus
extern "C"
{
#endif
    struct SEGGER_OFL_API
    {
        //
        // Optional functions may be be NULL
        //
        void (*pfFeedWatchdog)(void);                                                                  // Optional
        int (*pfInit)(uint32_t Addr, uint32_t Freq, uint32_t Func);                                    // Mandatory
        int (*pfUnInit)(uint32_t Func);                                                                // Mandatory
        int (*pfEraseSector)(uint32_t Addr);                                                           // Mandatory
        int (*pfProgramPage)(uint32_t Addr, uint32_t NumBytes, uint8_t *pSrcBuff);                     // Mandatory
        int (*pfBlankCheck)(uint32_t Addr, uint32_t NumBytes, uint8_t BlankData);                      // Optional
        int (*pfEraseChip)(void);                                                                      // Optional
        uint32_t (*pfVerify)(uint32_t Addr, uint32_t NumBytes, uint8_t *pSrcBuff);                     // Optional
        uint32_t (*pfSEGGERCalcCRC)(uint32_t CRC, uint32_t Addr, uint32_t NumBytes, uint32_t Polynom); // Optional
        int (*pfSEGGERRead)(uint32_t Addr, uint32_t NumBytes, uint8_t *pDestBuff);                     // Optional
        int (*pfSEGGERProgram)(uint32_t DestAddr, uint32_t NumBytes, uint8_t *pSrcBuff);               // Optional
        int (*pfSEGGERErase)(uint32_t SectorAddr, uint32_t SectorIndex, uint32_t NumSectors);          // Optional
        void (*pfSEGGERStart)(volatile struct SEGGER_OPEN_CMD_INFO *pInfo);                            // Optional
    };
    /** Initialize programming functions
        @param adr device base address
        @param clk clock frequency (Hz)
        @param fnc function code (1 - Erase, 2 - Program, 3 - Verify)
        @return 0 on success, an error code otherwise
     */
    int Init(uint32_t adr, uint32_t clk, uint32_t fnc);

    /** De-Initialize programming functions [optional]
        @param fnc function code (1 - Erase, 2 - Program, 3 - Verify)
        @return 0 on success, an error code otherwise
     */
    int UnInit(uint32_t fnc);

    /** Check region for erased memory
        @param adr address to start from
        @param sz the amount of memory to check
        @param pat the pattern of erased memory (usually 0xff)
        @return 0 on success, an error code otherwise
     */
    int BlankCheck(uint32_t adr, uint32_t sz, uint8_t pat);

    /** Perform a full chip erase
        @return 0 on success, an error code otherwise
     */
    int EraseChip(void);

    /** Erase a single sector of memory
        @param adr address of a sector to erase
        @return 0 on success, an error code otherwise
     */
    int EraseSector(uint32_t adr);

    /** Program data into memory
        @param adr address to start programming from
        @param sz the amount of data to program
        @param buf memory contents to be programmed
        @return 0 on success, an error code otherwise
     */
    int ProgramPage(uint32_t adr, uint32_t sz, uint8_t *buf);

    /** Verify contents in memory
        @param adr start address of the verification
        @param sz the amount of data to be verified
        @param buf memory contents to be compared against
        @return 0 on success, an error code otherwise
     */
    uint32_t Verify(uint32_t adr, uint32_t sz, uint8_t *buf);

    int SEGGER_OPEN_Read(uint32_t Addr, uint32_t NumBytes, uint8_t *pDestBuff);            // Optional
    int SEGGER_OPEN_Program(uint32_t DestAddr, uint32_t NumBytes, uint8_t *pSrcBuff);      // Optional
    int SEGGER_OPEN_Erase(uint32_t SectorAddr, uint32_t SectorIndex, uint32_t NumSectors); // Optional

#ifdef __cplusplus
}
#endif

#endif