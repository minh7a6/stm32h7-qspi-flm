#ifndef __BOARD_HPP
#define __BOARD_HPP

namespace Board {
    inline void configure_alt(GPIO_TypeDef *port, uint32_t pad, uint32_t af_num)
    {
        // Alternate mode
        port->MODER = (port->MODER & ~(3 << (pad * 2))) | (2 << (pad * 2));
        // Push Pull
        port->OTYPER = (port->OTYPER & ~(1 << (pad)));
        // Speed Medium
        port->OSPEEDR = (port->OSPEEDR & ~(3 << (pad * 2))) | (3 << (pad * 2));
        // No pull
        port->PUPDR = (port->PUPDR & ~(3 << (pad * 2)));
        // Configure to af mode
        if (pad < 8) {
            port->AFR[0] = (port->AFR[0] & ~(0xF << (pad * 4))) | (af_num << (pad * 4));
        }
        else {
            port->AFR[1] = (port->AFR[1] & ~(0xF << ((pad % 8) * 4))) | (af_num << ((pad % 8) * 4));
        }
    }
    // Set voltage to 400mhz
    inline void rcc_config()
    {
        RCC->APB4ENR |= RCC_APB4ENR_SYSCFGEN;
        MODIFY_REG(PWR->D3CR, PWR_D3CR_VOS, (PWR_D3CR_VOS_0 | PWR_D3CR_VOS_1));
        RCC->CR |= RCC_CR_HSION;
        while((RCC->CR & RCC_CR_HSIRDY) == 0);
        MODIFY_REG(RCC->PLLCKSELR, RCC_PLLCKSELR_PLLSRC, RCC_PLLCKSELR_PLLSRC_HSI);
        RCC->PLLCFGR |= RCC_PLLCFGR_DIVP1EN;
        MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLL1RGE, RCC_PLLCFGR_PLL1RGE_3);
        RCC->PLLCFGR &= RCC_PLLCFGR_PLL1VCOSEL;
        MODIFY_REG(RCC->PLLCKSELR, RCC_PLLCKSELR_DIVM1, RCC_PLLCKSELR_DIVM1_4);
        MODIFY_REG(RCC->PLL1DIVR, RCC_PLL1DIVR_N1, 50UL << RCC_PLL1DIVR_N1_Pos);
        MODIFY_REG(RCC->PLL1DIVR, RCC_PLL1DIVR_P1, 1UL << RCC_PLL1DIVR_P1_Pos);
        MODIFY_REG(RCC->PLL1DIVR, RCC_PLL1DIVR_Q1, 1UL << RCC_PLL1DIVR_Q1_Pos);
        MODIFY_REG(RCC->PLL1DIVR, RCC_PLL1DIVR_R1, 1UL << RCC_PLL1DIVR_R1_Pos);
        RCC->CR |= RCC_CR_PLL1ON;
        while((RCC->CR & RCC_CR_PLL1RDY) == 0);
        // Divide ahb to 2
        MODIFY_REG(RCC->D1CFGR, RCC_D1CFGR_HPRE, RCC_D1CFGR_HPRE_DIV2);
        MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_PLL1);
        while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL1);
        MODIFY_REG(RCC->D1CFGR, RCC_D1CFGR_D1CPRE, RCC_D1CFGR_D1CPRE_DIV1);
        MODIFY_REG(RCC->D1CFGR, RCC_D1CFGR_HPRE, RCC_D1CFGR_HPRE_DIV2);
        MODIFY_REG(RCC->D2CFGR, RCC_D2CFGR_D2PPRE1, RCC_D2CFGR_D2PPRE1_DIV2);
        MODIFY_REG(RCC->D2CFGR, RCC_D2CFGR_D2PPRE2, RCC_D2CFGR_D2PPRE2_DIV2);
        MODIFY_REG(RCC->D1CFGR, RCC_D1CFGR_D1PPRE, RCC_D1CFGR_D1PPRE_DIV2);
        MODIFY_REG(RCC->D3CFGR, RCC_D3CFGR_D3PPRE, RCC_D3CFGR_D3PPRE_DIV2);
    }
    inline void gpio_init()
    {
        // PD13, PD12, PD11, PB6, PB2, PE2
        RCC->AHB4ENR |= (RCC_AHB4ENR_GPIOEEN | RCC_AHB4ENR_GPIOBEN | RCC_AHB4ENR_GPIODEN);
        configure_alt(GPIOB, 2,  9);
        configure_alt(GPIOB, 6, 10);
        configure_alt(GPIOD, 11, 9);
        configure_alt(GPIOD, 12, 9);
        configure_alt(GPIOD, 13, 9);
        configure_alt(GPIOE, 2,  9);
    }

    inline void gpio_deinit()
    {
        // PD13, PD12, PD11, PB6, PB2, PE2
        RCC->AHB4ENR &= ~(RCC_AHB4ENR_GPIOEEN | RCC_AHB4ENR_GPIOBEN | RCC_AHB4ENR_GPIODEN);
    }
}

#endif