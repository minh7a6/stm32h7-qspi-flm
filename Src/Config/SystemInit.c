#include "stm32h7xx.h"

#define HWREG(x)    (*((volatile uint32_t *)(x)))
void SystemInit(void) {

    /* Enable the floating-point unit. Any configuration of the
    floating-point unit must be done here prior to it being enabled */
    HWREG(0xE000ED88) = ((HWREG(0xE000ED88) & ~0x00F00000) | 0x00F00000);

    /*------- Reset the RCC clock configuration to the default reset state -------*/
    /* Set HSION bit */
    RCC->CR |= 0x00000001;
    /* Reset CFGR register */
    RCC->CFGR = 0x00000000;
    /* Reset HSEON, CSSON , CSION,RC48ON, CSIKERON PLL1ON, PLL2ON and PLL3ON bits */
    RCC->CR &= (uint32_t)0xEAF6ED7F;
    /* Reset D1CFGR register */
    RCC->D1CFGR = 0x00000000;
    /* Reset D2CFGR register */
    RCC->D2CFGR = 0x00000000;
    /* Reset D3CFGR register */
    RCC->D3CFGR = 0x00000000;
    /* Reset PLLCKSELR register */
    RCC->PLLCKSELR = 0x00000000;
    /* Reset PLLCFGR register */
    RCC->PLLCFGR = 0x00000000;
    /* Reset PLL1DIVR register */
    RCC->PLL1DIVR = 0x00000000;
    /* Reset PLL1FRACR register */
    RCC->PLL1FRACR = 0x00000000;
    /* Reset PLL2DIVR register */
    RCC->PLL2DIVR = 0x00000000;
    /* Reset PLL2FRACR register */
    RCC->PLL2FRACR = 0x00000000;
    /* Reset PLL3DIVR register */
    RCC->PLL3DIVR = 0x00000000;
    /* Reset PLL3FRACR register */
    RCC->PLL3FRACR = 0x00000000;
    /* Reset HSEBYP bit */
    RCC->CR &= (uint32_t)0xFFFBFFFF;
    /* Disable all interrupts */
    RCC->CIER = 0x00000000;

    /* Change the switch matrix read issuing capability to 1 for the AXI SRAM target (Target 7) */
    HWREG(0x51008108) = 0x000000001;
}
