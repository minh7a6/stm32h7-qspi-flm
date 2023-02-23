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