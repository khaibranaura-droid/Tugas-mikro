#include "stm32f4xx.h"
#include <stdint.h>
#define PROJECT 1

#define LED1   (1U << 5)   // PA5
#define LED2   (1U << 6)   // PA6
#define LED3   (1U << 7)   // PA7
#define BUTTON (1U << 0)   // PB0

volatile uint8_t button_flag = 0;
volatile uint8_t debounce_flag = 0;

void GPIO_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;

    GPIOA->MODER &= ~(0x3FU << 10);
    GPIOA->MODER |=  (0x15U << 10);
    GPIOA->OTYPER &= ~(7U << 5);
    GPIOA->PUPDR &= ~(0x3FU << 10);

    GPIOB->MODER &= ~(3U << 0);
    GPIOB->PUPDR &= ~(3U << 0);
    GPIOB->PUPDR |=  (2U << 0);
}

void Project1_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN |
                    RCC_APB1ENR_TIM3EN |
                    RCC_APB1ENR_TIM4EN;
    TIM2->PSC = 15999;
    TIM2->ARR = 499;       
    TIM2->CNT = 0;
    TIM2->EGR = TIM_EGR_UG;
    TIM2->SR = 0;
    TIM2->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM2_IRQn);
    TIM2->CR1 |= TIM_CR1_CEN;

    TIM3->PSC = 15999;
    TIM3->ARR = 999;       
    TIM3->CNT = 0;
    TIM3->EGR = TIM_EGR_UG;
    TIM3->SR = 0;
    TIM3->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM3_IRQn);
    TIM3->CR1 |= TIM_CR1_CEN;

    TIM4->PSC = 15999;
    TIM4->ARR = 1999;     
    TIM4->CNT = 0;
    TIM4->EGR = TIM_EGR_UG;
    TIM4->SR = 0;
    TIM4->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM4_IRQn);
}

void Project2_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    SYSCFG->EXTICR[0] &= ~(0xFU << 0);
    SYSCFG->EXTICR[0] |=  (1U << 0);

    EXTI->RTSR |= (1U << 0);       
    EXTI->FTSR &= ~(1U << 0);
    EXTI->PR = (1U << 0);         
    EXTI->IMR |= (1U << 0);

    TIM2->PSC = 15999;
    TIM2->ARR = 19;
    TIM2->CNT = 0;
    TIM2->EGR = TIM_EGR_UG;
    TIM2->SR = 0;
    TIM2->DIER |= TIM_DIER_UIE;

    NVIC_EnableIRQ(EXTI0_IRQn);
    NVIC_EnableIRQ(TIM2_IRQn);
}

void Project3_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    TIM2->PSC = 15999;
    TIM2->ARR = 499;
    TIM2->CNT = 0;
    TIM2->EGR = TIM_EGR_UG;
    TIM2->SR = 0;
    TIM2->DIER |= TIM_DIER_UIE;

    NVIC_EnableIRQ(TIM2_IRQn);
}

void TIM2_IRQHandler(void)
{
    if (TIM2->SR & TIM_SR_UIF)
    {
        TIM2->SR &= ~TIM_SR_UIF;

#if PROJECT == 1
        GPIOA->ODR ^= LED1;

#elif PROJECT == 2
        TIM2->CR1 &= ~TIM_CR1_CEN;

        if (GPIOB->IDR & BUTTON)
        {
            button_flag = 1;
        }

        debounce_flag = 0;
        EXTI->PR = (1U << 0);
        EXTI->IMR |= (1U << 0);

#elif PROJECT == 3
        if (GPIOB->IDR & BUTTON)
        {
            GPIOA->ODR ^= LED1;
        }
        else
        {
            GPIOA->BSRR = (LED1 << 16);
        }
#endif
    }
}

#if PROJECT == 1

void TIM3_IRQHandler(void)
{
    if (TIM3->SR & TIM_SR_UIF)
    {
        TIM3->SR &= ~TIM_SR_UIF;
        GPIOA->ODR ^= LED2;
    }
}

void TIM4_IRQHandler(void)
{
    if (TIM4->SR & TIM_SR_UIF)
    {
        TIM4->SR &= ~TIM_SR_UIF;
        GPIOA->ODR ^= LED3;
    }
}

#elif PROJECT == 2

void EXTI0_IRQHandler(void)
{
    if (EXTI->PR & (1U << 0))
    {
        EXTI->PR = (1U << 0);

        if (debounce_flag == 0U)
        {
            debounce_flag = 1;
            EXTI->IMR &= ~(1U << 0);

            TIM2->CNT = 0;
            TIM2->SR = 0;
            TIM2->CR1 |= TIM_CR1_CEN;
        }
    }
}

#endif

int main(void)
{
    GPIO_Init();

#if PROJECT == 1
    Project1_Init();

#elif PROJECT == 2
    Project2_Init();

#elif PROJECT == 3
    Project3_Init();

#else
#error "PROJECT harus bernilai 1, 2, atau 3"
#endif

    while (1)
    {
#if PROJECT == 2
        if (button_flag)
        {
            button_flag = 0;
            GPIOA->ODR ^= LED1;
        }

#elif PROJECT == 3
        if (GPIOB->IDR & BUTTON)
        {
            TIM2->CR1 |= TIM_CR1_CEN;
        }
        else
        {
            TIM2->CR1 &= ~TIM_CR1_CEN;
            TIM2->CNT = 0;
            TIM2->SR = 0;
            GPIOA->BSRR = (LED1 << 16);
        }
#endif
    }
}
