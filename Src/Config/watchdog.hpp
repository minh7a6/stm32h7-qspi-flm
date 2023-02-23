#ifndef __WATCHDOG_HPP
#define __WATCHDOG_HPP

#include "stm32h7xx.h"
#include <optional>
#include <array>
namespace watchdog {
    constexpr uint32_t key_reload  = 0xAAAAU;
    constexpr uint32_t key_enable  = 0xCCCCU;
    constexpr uint32_t key_write   = 0x5555U;
    constexpr uint32_t key_protect = 0x0000U;
    enum presc {
        PRESC_4 = 0,
        PRESC_8,
        PRESC_16,
        PRESC_32,
        PRESC_64,
        PRESC_128,
        PRESC_256,
        PRESC_MAX,
    };
    struct init_t {
        uint32_t rlr;
        presc pr;
    };
    inline void init(const init_t &init_struct) {
        IWDG1->KR = key_enable;
        IWDG1->KR = key_write;
        IWDG1->PR = static_cast<uint32_t>(init_struct.pr);
        IWDG1->RLR = static_cast<uint32_t>(init_struct.rlr);
        while(IWDG1->SR != 0);
    }
    /**
     * @brief Refresh the watchdog
     * 
     */
    inline void refresh() {
        IWDG1->KR = key_reload;
    }
    /**
     * @brief helper function to get the watchdog configuration based on timeout
     * 
     * @param ms milisecond timeout
     * @return init_t struct used for init
     */
    constexpr std::optional<init_t> get_init(uint32_t ms) {
        constexpr uint32_t ref_clk = 32000; // ref_clk
        constexpr uint32_t max_rlr = (2 << 12) - 1; // max_rlr
        std::array<presc, PRESC_MAX> range = {
            PRESC_4,
            PRESC_8,
            PRESC_16,
            PRESC_32,
            PRESC_64,
            PRESC_128,
            PRESC_256,
        };
        for(presc i : range) {
            uint32_t pr = 4UL * (2 << i);
            uint32_t rlr = (ms * ref_clk) / (pr * 1000UL);
            if ((rlr != 0) && ((rlr-1) < max_rlr)) {
                init_t val = {rlr - 1, i};
                return val;
            }
        }
        return std::nullopt;
    }
}

#endif