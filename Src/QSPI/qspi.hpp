#ifndef QSPI_H
#define QSPI_H
#include <stdint.h>
#include "stm32h7xx.h"
class qspi_driver
{
public:
    enum error_t
    {
        QSPI_OK = 0,
        QSPI_TIME_OUT,
        QSPI_HARDWARE_ERROR
    };
    enum cmd_data_mode
    {
        QSPI_None = 0,
        QSPI_1_LINE,
        QSPI_2_LINE,
        QSPI_4_LINE
    };
    enum ddr_hhr_delay
    {
        ANALOG_DELAY = 0,
        HALF_CLK_DELAY,
    };
    enum clk_mode
    {
        SDR = 0, // Single Data rate
        DDR      // Double data rate
    };
    enum match_t
    {
        AND = 0,
        OR
    };
    enum alter_ad_size
    {
        L8B = 0, // 8 bits
        L16B,    // 16 bits
        L24B,    // 24 bits
        L32B,    // 32 bits
    };
    enum fmode
    {
        INDIRECT_WRITE = 0,
        INDIRECT_READ,
        AUTO_POLL,
        MEM_MAP
    };
    struct instruction_t
    {
        cmd_data_mode mode;
        uint8_t cmd;
    };
    struct address_t
    {
        cmd_data_mode mode;
        alter_ad_size size;
        uint32_t address;
    };
    struct alter_byte_t
    {
        cmd_data_mode mode;
        alter_ad_size size;
        uint32_t alternate_bytes;
    };
    struct ddr_mode_t
    {
        clk_mode mode;
        ddr_hhr_delay delay;
    };
    struct data_t
    {
        cmd_data_mode mode;
        uint8_t *buf;
        uint32_t size;
    };
    struct header_t
    {
        instruction_t instruction;
        address_t address;
        alter_byte_t alternative_byte;
        ddr_mode_t ddr;
        uint8_t dummy_cycles;
        bool sio0;
    };
    struct transact_t
    {
        const header_t header;
        data_t data;
    };
    struct memmap_t
    {
        const header_t header;
        struct memmap_per
        {
            cmd_data_mode mode;
            uint32_t period;
            bool is_timeout;
        } memmap;
    };
    struct polling_t
    {
        const header_t header;
        struct poll_mask
        {
            uint32_t match;
            uint32_t mask;
            uint32_t size;
            uint16_t interval;
            match_t match_mode;
            bool autostop;
            cmd_data_mode mode;
        } poll;
    };
    struct init_t
    {
        uint8_t presc;              // Prescaler
        uint8_t fifo_thresh;        // threshold
        uint8_t fsize;              // FSIZE
        uint8_t chip_sel_high_time; // Chip Sel high time
        bool ckmode;                // ckmode
        bool sample_shift;          // sample shift
    };
    void init(const init_t &init_val)
    {
        RCC->AHB3ENR |= RCC_AHB3ENR_QSPIEN; // Enable QUADSPI Clk
        _ptr->DCR = (QUADSPI->DCR & ~(QUADSPI_DCR_FSIZE | QUADSPI_DCR_CSHT | QUADSPI_DCR_CKMODE)) |
                       ((static_cast<uint32_t>(init_val.fsize) << QUADSPI_DCR_FSIZE_Pos) |
                        (static_cast<uint32_t>(init_val.chip_sel_high_time) << QUADSPI_DCR_CSHT_Pos) |
                        (static_cast<uint32_t>(init_val.ckmode) << QUADSPI_DCR_CKMODE_Pos));
        _ptr->CR = (QUADSPI->CR & ~(QUADSPI_CR_PRESCALER | QUADSPI_CR_DFM | QUADSPI_CR_FTHRES)) |
                      ((static_cast<uint32_t>(init_val.sample_shift) << QUADSPI_CR_SSHIFT_Pos) |
                       (static_cast<uint32_t>(init_val.presc) << QUADSPI_CR_PRESCALER_Pos) |
                       (static_cast<uint32_t>(init_val.fifo_thresh) << QUADSPI_CR_FTHRES_Pos));
        _ptr->CR |= QUADSPI_CR_EN;
    }
    void deinit()
    {
        _ptr->CR &= ~QUADSPI_CR_EN;
        RCC->AHB3ENR &= ~RCC_AHB3ENR_QSPIEN;
    }
    error_t abort();
    error_t mmap(const qspi_driver::memmap_t &transaction);
    error_t poll(const polling_t &poll);
    error_t write(const transact_t &transaction);
    error_t read(transact_t &transaction);
    static constexpr uint8_t get_fsize(uint32_t value)
    {
        const int tab32[32] = {
            0, 9, 1, 10, 13, 21, 2, 29,
            11, 14, 16, 18, 22, 25, 3, 30,
            8, 12, 20, 28, 15, 17, 24, 7,
            19, 27, 23, 6, 26, 5, 4, 31};
        value |= value >> 1;
        value |= value >> 2;
        value |= value >> 4;
        value |= value >> 8;
        value |= value >> 16;
        return static_cast<uint8_t>(tab32[(uint32_t)(value * 0x07C4ACDD) >> 27] - 1);
    }
    qspi_driver(QUADSPI_TypeDef *ptr) : _ptr(ptr) {}

private:
    void set_header(const header_t &header, cmd_data_mode data_mode, fmode mode);
    error_t check_error();
    QUADSPI_TypeDef *_ptr;
};

#endif