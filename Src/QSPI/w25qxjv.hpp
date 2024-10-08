#ifndef W25Q64JV_HPP
#define W25Q64JV_HPP

#include "QspiFlash.hpp"
#include <array>

template <uint32_t flash_sz>
class w25qxjv : public QspiFlash
{
public:
    w25qxjv(qspi_driver &drv) : QspiFlash(drv) {}
    int init()
    {
        auto res = restart();
        if (res != 0)
        {
            return res;
        }
        /* Enable Quad SPI for the chip */
        return enable_qio();
    }
    int program_page(void *dest, const uint32_t size, void *src)
    {
        uint32_t dst_addr = reinterpret_cast<uint32_t>(dest);
        const qspi_driver::transact_t prg_cmd = {
            {
                {qspi_driver::QSPI_1_LINE, quad_in_fast_prog},           // instruction
                {qspi_driver::QSPI_1_LINE, qspi_driver::L24B, dst_addr}, // address
                {qspi_driver::QSPI_None, qspi_driver::L24B, 0},          // alternate bytes
                {qspi_driver::SDR, qspi_driver::ANALOG_DELAY},           // ddr mode
                0,                                                       // dummy cycle
                false                                                    // sio0
            },
            {qspi_driver::QSPI_4_LINE, static_cast<uint8_t *>(src), size},
        };
        // Enable write
        int res = wen();
        if (res != qspi_driver::QSPI_OK)
        {
            return res;
        }
        res = _drv.write(prg_cmd);
        if (res != qspi_driver::QSPI_OK)
        {
            return res;
        }
        return poll_busy();
        return 0;
    }
    uint32_t verify(void *dest, const uint32_t size, void *src)
    {
        /* create buffer enough for a page */
        std::array<uint8_t, get_pg()> buffer;
        uint8_t *srcAddr = static_cast<uint8_t *>(src);
        uint32_t sz = 0;
        while (sz < size)
        {
            std::size_t read_sz = size;
            if (size > get_pg())
            {
                read_sz = get_pg();
            }
            int res = read(dest, read_sz, buffer.data());
            if (res != qspi_driver::QSPI_OK)
            {
                return 0;
            }
            for (std::size_t i = 0; i < read_sz; i++)
            {
                if (srcAddr[i + sz] != buffer[i])
                {
                    return sz;
                }
            }
            sz += read_sz;
        }
        return sz;
    }
    int blank_check(void *dest, const uint32_t size, uint8_t data)
    {
        /* create buffer enough for a page */
        std::array<uint8_t, get_pg()> buffer;
        uint32_t sz = 0;
        while (sz < size)
        {
            std::size_t read_sz = size;
            if (size > get_pg())
            {
                read_sz = get_pg();
            }
            int res = read(dest, read_sz, buffer.data());
            if (res != qspi_driver::QSPI_OK)
            {
                return res;
            }
            for (std::size_t i = 0; i < read_sz; i++)
            {
                if (data != buffer[i])
                {
                    return 1;
                }
            }
            sz += read_sz;
        }
        return 0;
    }

    int erase_sector(void *adr)
    {
        // Enable write
        int res = wen();
        if (res != qspi_driver::QSPI_OK)
        {
            return res;
        }
        uint32_t addr = reinterpret_cast<uint32_t>(adr);
        const qspi_driver::transact_t erase_cmd = {
            {
                {qspi_driver::QSPI_1_LINE, sector_erase},            // instruction
                {qspi_driver::QSPI_1_LINE, qspi_driver::L24B, addr}, // address
                {qspi_driver::QSPI_None, qspi_driver::L24B, 0},      // alternate bytes
                {qspi_driver::SDR, qspi_driver::ANALOG_DELAY},       // ddr mode
                0,                                                   // dummy cycle
                false                                                // sio0
            },
            {qspi_driver::QSPI_None, nullptr, 0},
        };
        res = _drv.write(erase_cmd);
        if (res != qspi_driver::QSPI_OK)
        {
            return res;
        }
        return poll_busy();
    }

    int erase_chip()
    {
        // Enable write
        int res = wen();
        if (res != qspi_driver::QSPI_OK)
        {
            return res;
        }
        const qspi_driver::transact_t erase_cmd = {
            {
                {qspi_driver::QSPI_1_LINE, chip_erase},         // instruction
                {qspi_driver::QSPI_None, qspi_driver::L24B, 0}, // address
                {qspi_driver::QSPI_None, qspi_driver::L24B, 0}, // alternate bytes
                {qspi_driver::SDR, qspi_driver::ANALOG_DELAY},  // ddr mode
                0,                                              // dummy cycle
                false                                           // sio0
            },
            {qspi_driver::QSPI_None, nullptr, 0},
        };
        res = _drv.write(erase_cmd);
        if (res != qspi_driver::QSPI_OK)
        {
            return res;
        }
        return poll_busy();
    }
    int read(void *dest, uint32_t size, void *buff)
    {
        uint32_t dst_addr = reinterpret_cast<uint32_t>(dest);

        qspi_driver::transact_t rd_cmd = {
            {
                {qspi_driver::QSPI_1_LINE, quad_out_fast_read},               // instruction
                {qspi_driver::QSPI_4_LINE, qspi_driver::L24B, dst_addr},      // address
                {qspi_driver::QSPI_4_LINE, qspi_driver::L8B, alternate_byte}, // alternate bytes
                {qspi_driver::SDR, qspi_driver::ANALOG_DELAY},                // ddr mode
                4,                                                            // dummy cycle
                false                                                         // sio0
            },
            {qspi_driver::QSPI_4_LINE, static_cast<uint8_t *>(buff), size},
        };
        auto res = _drv.read(rd_cmd);
        if (res != qspi_driver::QSPI_OK)
        {
            return res;
        }
        return 0;
    }
    int mmap()
    {
        const qspi_driver::memmap_t memmap_cmd = {
            {
                {qspi_driver::QSPI_1_LINE, quad_out_fast_read},               // instruction
                {qspi_driver::QSPI_4_LINE, qspi_driver::L24B, 0},             // address
                {qspi_driver::QSPI_4_LINE, qspi_driver::L8B, alternate_byte}, // alternate bytes
                {qspi_driver::SDR, qspi_driver::ANALOG_DELAY},                // ddr mode
                4,                                                            // dummy cycle
                false                                                         // sio0
            },
            {qspi_driver::QSPI_4_LINE, 0, false},
        };
        return _drv.mmap(memmap_cmd);
    }
    static constexpr uint32_t get_size() { return size; }
    static constexpr uint32_t get_pg() { return pg_size; }
    static constexpr uint32_t get_sect_size() { return sector_size; }
    static constexpr uint32_t get_max_clk() { return clk; }

private:
    int enable_qio()
    {
        uint8_t reg = 0;
        qspi_driver::transact_t get_status = {
            {
                {qspi_driver::QSPI_1_LINE, read_conf_reg},      // instruction
                {qspi_driver::QSPI_None, qspi_driver::L24B, 0}, // address
                {qspi_driver::QSPI_None, qspi_driver::L24B, 0}, // alternate bytes
                {qspi_driver::SDR, qspi_driver::ANALOG_DELAY},  // ddr mode
                0,                                              // dummy cycle
                false                                           // sio0
            },
            {qspi_driver::QSPI_1_LINE, &reg, sizeof(reg)},
        };
        auto res = _drv.read(get_status);
        if (res != qspi_driver::QSPI_OK)
        {
            return res;
        }
        if (!((reg & 0x2) >> 1))
        {
            reg |= 0x2;
            auto res = wen();
            if (res != qspi_driver::QSPI_OK)
            {
                return res;
            }
            const qspi_driver::transact_t write_val = {
                {{qspi_driver::QSPI_1_LINE, static_cast<uint32_t>(write_vol_cfg_reg)}, // instruction
                 {qspi_driver::QSPI_None, qspi_driver::L24B, 0},                       // address
                 {qspi_driver::QSPI_None, qspi_driver::L24B, 0},                       // alternate bytes
                 {qspi_driver::SDR, qspi_driver::ANALOG_DELAY},                        // ddr mode
                 0,
                 false},
                {qspi_driver::QSPI_1_LINE, &reg, 1},
            };
            return _drv.write(write_val);
        }
        return 0;
    }

    int restart()
    {
        /* Enable Reset */
        const qspi_driver::transact_t rst_en = {
            {
                {qspi_driver::QSPI_1_LINE, reset_enable},       // instruction
                {qspi_driver::QSPI_None, qspi_driver::L24B, 0}, // address
                {qspi_driver::QSPI_None, qspi_driver::L24B, 0}, // alternate bytes
                {qspi_driver::SDR, qspi_driver::ANALOG_DELAY},  // ddr mode
                0,                                              // dummy cycle
                false                                           // sio0
            },
            {qspi_driver::QSPI_None, nullptr, 0},
        };
        auto res = _drv.write(rst_en);
        if (res != qspi_driver::QSPI_OK)
        {
            return res;
        }

        /* Actually Reset the chip */
        const qspi_driver::transact_t rst_exc = {
            {{qspi_driver::QSPI_1_LINE, reset_execute},      // instruction
             {qspi_driver::QSPI_None, qspi_driver::L24B, 0}, // address
             {qspi_driver::QSPI_None, qspi_driver::L24B, 0}, // alternate bytes
             {qspi_driver::SDR, qspi_driver::ANALOG_DELAY},  // ddr mode
             0,
             false},
            {qspi_driver::QSPI_None, nullptr, 0},
        };
        res = _drv.write(rst_exc);
        if (res != 0)
        {
            return res;
        }
        return poll_busy();
    }

    int wen()
    {
        const qspi_driver::transact_t wen_exc = {
            {
                {qspi_driver::QSPI_1_LINE, write_enable},       // instruction
                {qspi_driver::QSPI_None, qspi_driver::L24B, 0}, // address
                {qspi_driver::QSPI_None, qspi_driver::L24B, 0}, // alternate bytes
                {qspi_driver::SDR, qspi_driver::ANALOG_DELAY},  // ddr mode
                0,                                              // dummy cycle
                false                                           // sio0
            },
            {qspi_driver::QSPI_None, nullptr, 0},
        };
        auto res = _drv.write(wen_exc);
        if (res != qspi_driver::QSPI_OK)
        {
            return res;
        }
        /* Poll the status bit 2 for enabling the write */
        const qspi_driver::polling_t poll_trans = {
            {{qspi_driver::QSPI_1_LINE, read_status_reg},    // instruction
             {qspi_driver::QSPI_None, qspi_driver::L24B, 0}, // address
             {qspi_driver::QSPI_None, qspi_driver::L24B, 0}, // alternate bytes
             {qspi_driver::SDR, qspi_driver::ANALOG_DELAY},  // ddr mode
             0,
             false},
            /* the eqn will be (reg % mask) = match */
            {
                0x02,                    // match res
                0x02,                    // mask
                0x01,                    // byte size
                0x10,                    // interval
                qspi_driver::AND,        // match mode
                true,                    // auto stop
                qspi_driver::QSPI_1_LINE // 1 Line
            }};
        return _drv.poll(poll_trans);
    }

    int poll_busy()
    {
        const qspi_driver::polling_t poll_trans = {
            {{qspi_driver::QSPI_1_LINE, read_status_reg},    // instruction
             {qspi_driver::QSPI_None, qspi_driver::L24B, 0}, // address
             {qspi_driver::QSPI_None, qspi_driver::L24B, 0}, // alternate bytes
             {qspi_driver::SDR, qspi_driver::ANALOG_DELAY},  // ddr mode
             0,
             false},
            {                  // the eqn will be (reg % mask) = match
             0x00,             // match res
             0x01,             // mask
             0x01,             // byte size
             0x10,             // interval
             qspi_driver::AND, // match mode
             true,             // auto stop
             qspi_driver::QSPI_1_LINE},
        };
        return _drv.poll(poll_trans);
    }

    static constexpr uint32_t size = flash_sz;
    static constexpr uint32_t pg_size = 0x100;
    static constexpr uint32_t sector_size = 0x00010000;
    static constexpr uint8_t alternate_byte = 0xf0;
    static constexpr uint32_t clk = 120000000UL;
    enum cmd
    {
        write_enable = 0x06,
        read_status_reg = 0x05,
        write_vol_cfg_reg = 0x31,
        sector_erase = 0xd8,
        chip_erase = 0xc7,
        quad_in_fast_prog = 0x32,
        read_conf_reg = 0x35,
        quad_out_fast_read = 0xeb,
        reset_enable = 0x66,
        reset_execute = 0x99,
    };
};

class w25q64jv final : public w25qxjv<0x800000> {
public:
    w25q64jv(qspi_driver &drv) : w25qxjv<0x800000>(drv) {}
};

class w25q32jv final : public w25qxjv<0x400000> {
public:
    w25q32jv(qspi_driver &drv) : w25qxjv<0x400000>(drv) {}
};

class w25q16jv final : public w25qxjv<0x200000> {
public:
    w25q16jv(qspi_driver &drv) : w25qxjv<0x200000>(drv) {}
};

#endif