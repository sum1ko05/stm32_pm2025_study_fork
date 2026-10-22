#include <stdint.h>
#include <stm32f10x.h>

#define Millisecond 1000000U
#define TimeChunk 1562500U

void delay(uint32_t ticks) {
	for (int i=0; i<ticks; i++) {
		__NOP();
	}
}

int __attribute((noreturn)) main(void) {
	// Enable clock for AFIO
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
	// Enable clock for GPIOC
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
	
	// Initial port settings
	// PC13 - 10Mhz output
	GPIOC->CRH &= ~(GPIO_CRH_CNF13 | GPIO_CRH_MODE13); 
	GPIOC->CRH |= GPIO_CRH_MODE13_0;
	// PC14 - pull-up input
	GPIOC->CRH &= ~(GPIO_CRH_CNF14 | GPIO_CRH_MODE14);
	GPIOC->CRH |= GPIO_CRH_CNF14_1;
	// PC15 - pull-up input
	GPIOC->CRH &= ~(GPIO_CRH_CNF15 | GPIO_CRH_MODE15);
	GPIOC->CRH |= GPIO_CRH_CNF15_1;

	uint32_t i;
	uint32_t period = 1U<<6U;
	//uint32_t period = 1000U;

	uint16_t buttons_state;
    while (1) {
		// Activating PC14 divides period by 2 (min period is ~1/8 seconds)
		if((GPIOC->IDR & GPIO_IDR_IDR14) && (~buttons_state & GPIO_IDR_IDR14))
		{
			buttons_state |= GPIO_IDR_IDR14; // Set PC14 button state
			if(period > 1U<<3U) period >>= 1U;
		}
		else if((~GPIOC->IDR & GPIO_IDR_IDR14)) buttons_state &= ~GPIO_IDR_IDR14; // Reset PC14 button state

		// Activating PC15 multiplies period by 2 (max period is ~8 seconds)
		if((GPIOC->IDR & GPIO_IDR_IDR15) && (~buttons_state & GPIO_IDR_IDR15))
		{
			buttons_state |= GPIO_IDR_IDR15; // Set PC15 button state
			if(period < 1U<<9U) period <<= 1U;
		}
		else if((~GPIOC->IDR & GPIO_IDR_IDR15)) buttons_state &= ~GPIO_IDR_IDR15; // Reset PC15 button state
		
		// Hold board only for a TimeChunk ticks to adequately read inputs
		if(i >= period)
		{
			GPIOC->ODR ^= GPIO_ODR_ODR13;
			i=0;
		}
		i++;
		delay(TimeChunk);
    }
}
