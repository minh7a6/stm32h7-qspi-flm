/*
 * Dev_Inf.c
 *
 */
#include "Dev_Inf.hpp"
#include "Config.hpp"
extern "C"
{
    struct StorageInfo const StorageInfo __attribute__((used)) = {
        FLASH_LDR_NAME, // Device Name + version number
        SPI_FLASH,      // Device Type
        QSPI_BASE,      // Device Start Address
        flash_size,     // Device Size in Bytes
        pg_size,        // Programming Page Size
        0xFF,           // Initial Content of Erased Memory

        // Specify Size and Address of Sectors (view example below)
        (flash_size / sector_size), (uint32_t)sector_size, // Sector Size
        0x00000000, 0x00000000};
};