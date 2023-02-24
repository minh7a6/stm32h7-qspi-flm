
#include "Config.hpp"
#include "watchdog.hpp"
#include "Board.hpp"
#define LOADER_OK 0x1
#define LOADER_FAIL 0x0

extern "C"
{
    void SystemInit(void);
};

extern "C"
{
    /**
     * @brief  System initialization.
     * @param  None
     * @retval  LOADER_OK = 1   : Operation succeeded
     * @retval  LOADER_FAIL = 0 : Operation failed
     */
    int Init(void)
    {
        SystemInit();
        SCB_InvalidateICache();
        SCB_InvalidateDCache();
        SCB_EnableICache();
        SCB_EnableDCache();
        // constexpr auto watchdog_init = watchdog::get_init(5000);
        qspi_driver drv(QUADSPI);
        FLASH_CLASS flash(drv);
        drv.deinit();
        Board::gpio_deinit();
        Board::gpio_init();
        drv.init(qspi_init);
        int res = flash.init();
        if (res != 0)
        {
            return LOADER_FAIL;
        }
        // watchdog::refresh();
        if (flash.mmap() != 0)
        {
            return LOADER_FAIL;
        }
        /* Quirks: Trigger Read access, otherwise abort will stuck */
        uint32_t a = *(uint32_t *)(QSPI_BASE);
        a++;
        return LOADER_OK;
    }

    /**
     * @brief   Program memory.
     * @param   Address: page address
     * @param   Size   : size of data
     * @param   buffer : pointer to data buffer
     * @retval  LOADER_OK = 1       : Operation succeeded
     * @retval  LOADER_FAIL = 0 : Operation failed
     */
    int Write(uint32_t Address, uint32_t Size, uint8_t *buffer)
    {
        int res;
        Address -= QSPI_BASE;
        // watchdog::refresh();
        qspi_driver drv(QUADSPI);
        FLASH_CLASS flash(drv);
        drv.deinit();
        Board::gpio_deinit();
        Board::gpio_init();
        drv.init(qspi_init);
        res = flash.init();
        if (res != 0)
        {
            return LOADER_FAIL;
        }
        const auto pg_offset = Address % flash.get_pg();
        if (pg_offset != 0 && Size > flash.get_pg())
        {
            if (flash.program_page(reinterpret_cast<void *>(Address), flash.get_pg() - pg_offset, buffer) != 0)
            {
                return LOADER_FAIL;
            }
            buffer += flash.get_pg() - pg_offset;
            Address += flash.get_pg() - pg_offset;
            Size -= flash.get_pg() - pg_offset;
        }
        auto num_pages = Size / flash.get_pg();
        while (num_pages > 0)
        {
            if (flash.program_page(reinterpret_cast<void *>(Address), flash.get_pg(), buffer) != 0)
            {
                return LOADER_FAIL;
            }
            buffer += flash.get_pg();
            Address += flash.get_pg();
            num_pages -= 1;
            Size -= flash.get_pg();
        }
        if (Size > 0)
        {
            if (flash.program_page(reinterpret_cast<void *>(Address), Size, buffer) != 0)
            {
                return LOADER_FAIL;
            }
        }
        return LOADER_OK;
    }

    /**
     * @brief   Sector erase.
     * @param   EraseStartAddress :  erase start address
     * @param   EraseEndAddress   :  erase end address
     * @retval  LOADER_OK = 1       : Operation succeeded
     * @retval  LOADER_FAIL = 0 : Operation failed
     */
    int SectorErase(uint32_t EraseStartAddress, uint32_t EraseEndAddress)
    {
        EraseStartAddress -= QSPI_BASE;
        EraseEndAddress -= QSPI_BASE;
        // watchdog::refresh();
        qspi_driver drv(QUADSPI);
        FLASH_CLASS flash(drv);
        int res;
        drv.deinit();
        Board::gpio_deinit();
        Board::gpio_init();
        drv.init(qspi_init);
        res = flash.init();
        if (res != 0)
        {
            return LOADER_FAIL;
        }
        while (EraseEndAddress >= EraseStartAddress)
        {
            void *addr = reinterpret_cast<void *>(EraseStartAddress);
            res = flash.erase_sector(addr);
            if (res != 0)
            {
                return LOADER_FAIL;
            }
            EraseStartAddress += flash.get_sect_size();
        }
        return LOADER_OK;
    }

    /**
     * Description :
     * Mass erase of external flash area
     * Optional command - delete in case usage of mass erase is not planed
     * Inputs    :
     *      none
     * outputs   :
     *     none
     * Note: Optional for all types of device
     */
    int MassErase(void)
    {
        // watchdog::refresh();
        qspi_driver drv(QUADSPI);
        FLASH_CLASS flash(drv);
        int res;
        // res = flash.abort();
        // if (res != 0)
        // {
        //     return LOADER_FAIL;
        // }
        drv.deinit();
        Board::gpio_deinit();
        Board::gpio_init();
        drv.init(qspi_init);
        res = flash.init();
        if (res != 0)
        {
            return LOADER_FAIL;
        }
        res = flash.erase_chip();
        if (res != 0)
        {
            return LOADER_FAIL;
        }
        return LOADER_OK;
    }

    /**
     * Description :
     * Calculates checksum value of the memory zone
     * Inputs    :
     *      StartAddress  : Flash start address
     *      Size          : Size (in WORD)
     *      InitVal       : Initial CRC value
     * outputs   :
     *     R0             : Checksum value
     * Note: Optional for all types of device
     */
    uint32_t
    CheckSum(uint32_t StartAddress, uint32_t Size, uint32_t InitVal)
    {
        uint8_t missalignementAddress = StartAddress % 4;
        uint8_t missalignementSize = Size;
        uint32_t cnt;
        uint32_t Val;

        StartAddress -= StartAddress % 4;
        Size += (Size % 4 == 0) ? 0 : 4 - (Size % 4);

        for (cnt = 0; cnt < Size; cnt += 4)
        {
            Val = *(uint32_t *)StartAddress;
            if (missalignementAddress)
            {
                switch (missalignementAddress)
                {
                case 1:
                    InitVal += (uint8_t)(Val >> 8 & 0xff);
                    InitVal += (uint8_t)(Val >> 16 & 0xff);
                    InitVal += (uint8_t)(Val >> 24 & 0xff);
                    missalignementAddress -= 1;
                    break;
                case 2:
                    InitVal += (uint8_t)(Val >> 16 & 0xff);
                    InitVal += (uint8_t)(Val >> 24 & 0xff);
                    missalignementAddress -= 2;
                    break;
                case 3:
                    InitVal += (uint8_t)(Val >> 24 & 0xff);
                    missalignementAddress -= 3;
                    break;
                }
            }
            else if ((Size - missalignementSize) % 4 && (Size - cnt) <= 4)
            {
                switch (Size - missalignementSize)
                {
                case 1:
                    InitVal += (uint8_t)Val;
                    InitVal += (uint8_t)(Val >> 8 & 0xff);
                    InitVal += (uint8_t)(Val >> 16 & 0xff);
                    missalignementSize -= 1;
                    break;
                case 2:
                    InitVal += (uint8_t)Val;
                    InitVal += (uint8_t)(Val >> 8 & 0xff);
                    missalignementSize -= 2;
                    break;
                case 3:
                    InitVal += (uint8_t)Val;
                    missalignementSize -= 3;
                    break;
                }
            }
            else
            {
                InitVal += (uint8_t)Val;
                InitVal += (uint8_t)(Val >> 8 & 0xff);
                InitVal += (uint8_t)(Val >> 16 & 0xff);
                InitVal += (uint8_t)(Val >> 24 & 0xff);
            }
            StartAddress += 4;
        }

        return (InitVal);
    }

    /**
     * Description :
     * Verify flash memory with RAM buffer and calculates checksum value of
     * the programmed memory
     * Inputs    :
     *      FlashAddr     : Flash address
     *      RAMBufferAddr : RAM buffer address
     *      Size          : Size (in WORD)
     *      InitVal       : Initial CRC value
     * outputs   :
     *     R0             : Operation failed (address of failure)
     *     R1             : Checksum value
     * Note: Optional for all types of device
     */
    uint64_t
    Verify(uint32_t MemoryAddr, uint32_t RAMBufferAddr, uint32_t Size, uint32_t missalignement)
    {
        // watchdog::refresh();
        qspi_driver drv(QUADSPI);
        FLASH_CLASS flash(drv);
        uint32_t VerifiedData = 0, InitVal = 0;
        uint64_t checksum;
        Size *= 4;

        checksum = CheckSum((uint32_t)MemoryAddr + (missalignement & 0xf),
                            Size - ((missalignement >> 16) & 0xF), InitVal);
        while (Size > VerifiedData)
        {
            if (*(uint8_t *)MemoryAddr++ != *((uint8_t *)RAMBufferAddr + VerifiedData))
            {
                return ((checksum << 32) + (MemoryAddr + VerifiedData));
            }
            VerifiedData++;
        }
        return (checksum << 32);
    }

    // int Read (uint32_t Address, uint32_t Size, uint16_t* buffer)
    // {
    //     qspi_driver drv(QUADSPI);
    //     FLASH_CLASS flash(drv);
    //     if (flash.mmap() != 0)
    //     {
    //         return LOADER_FAIL;
    //     }
    //     uint16_t *addr = reinterpret_cast<uint16_t *> (Address);
    //     for (uint32_t i = 0; i < Size; i++) {
    //         buffer[i] = addr[i];
    //     }
    //     return LOADER_OK;
    // }
};