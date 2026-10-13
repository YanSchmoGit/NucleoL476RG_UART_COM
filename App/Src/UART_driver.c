//
// Created by yan on 10/9/26.
//

#include "../Int/UART_driver.h"

#include <string.h>

#include "stm32l476xx.h"

uint8_t InitGPIOPort(GPIO_TypeDef* GPIOPort, uint8_t GPIO_Pin_TX, uint8_t GPIO_Pin_RX)
{
    // Configure port
    if (GPIOPort == GPIOA)
    {
        RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    }
    else if (GPIOPort == GPIOB)
    {
        RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    }
    else if (GPIOPort == GPIOC)
    {
        RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
    }
    else if (GPIOPort == GPIOD)
    {
        RCC->AHB2ENR |= RCC_AHB2ENR_GPIODEN;
    }
    else
    {
        return 1;
    }

    // Configure GPIO

    // Set alternate function - AF7
    if (GPIO_Pin_RX <= 7)
    {
        GPIOPort->AFR[0] &= ~(0xFUL << (GPIO_Pin_RX * 4));
        GPIOPort->AFR[0] |= (0x7UL << (GPIO_Pin_RX * 4));
    }
    else if (GPIO_Pin_RX > 7)
    {
        GPIOPort->AFR[1] &= ~(0xFUL << ((GPIO_Pin_RX - 8) * 4));
        GPIOPort->AFR[1] |= (0x7UL << ((GPIO_Pin_RX - 8) * 4));
    }
    else
    {
        return 2;
    }

    if (GPIO_Pin_TX <= 7)
    {
        GPIOPort->AFR[0] &= ~(0xFUL << (GPIO_Pin_TX * 4));
        GPIOPort->AFR[0] |= (0x7UL << (GPIO_Pin_TX * 4));
    }
    else if (GPIO_Pin_TX > 7)
    {
        GPIOPort->AFR[1] &= ~(0xFUL << ((GPIO_Pin_TX - 8) * 4));
        GPIOPort->AFR[1] |= (0x7UL << ((GPIO_Pin_TX - 8) * 4));
    }
    else
    {
        return 2;
    }

    // PUPDR - Pull up TX/ Pull up RX
    GPIOPort->PUPDR &= ~((0x3UL << (GPIO_Pin_TX * 2)) | (0x3UL << (GPIO_Pin_RX * 2))); // Reset PUPDR
    GPIOPort->PUPDR |= (0x2UL << (GPIO_Pin_TX * 2)) | (0x2UL << (GPIO_Pin_RX * 2)); // Set to pull-up

    // OSPEEDR - TX configuration only
    GPIOPort->OSPEEDR &= ~(0x3UL << (GPIO_Pin_TX * 2)); // Reset pin speed
    GPIOPort->OSPEEDR |= (0x1UL << (GPIO_Pin_TX * 2));

    // MODER - alternate function mode
    GPIOPort->MODER &= ~((0x3UL << (GPIO_Pin_TX * 2)) | (0x3UL << (GPIO_Pin_RX * 2))); // Reset MODER
    GPIOPort->MODER |= (0x2UL << (GPIO_Pin_TX * 2)) | (0x2UL << (GPIO_Pin_RX * 2)); // Set alternate function mode


    return 0;
}

void UART_init(const USARTHandler_t* USARTHandle)
{
    // Init GPIO
    InitGPIOPort(USARTHandle->GPIOPort, USARTHandle->GPIO_Pin_TX, USARTHandle->GPIO_Pin_RX);

    // Init UART
    if (USARTHandle->USARTx == USART1)
    {
        RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    }
    else if (USARTHandle->USARTx == USART2)
    {
        RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
    }
    else if (USARTHandle->USARTx == USART3)
    {
        RCC->APB1ENR1 |= RCC_APB1ENR1_USART3EN;
    }

    // Configure USART

    // Bauderate - Set bauderate to handler value
    uint32_t systemClock = SystemCoreClock;
    USARTHandle->USARTx->BRR = (uint32_t)systemClock / USARTHandle->BaudRate;

    USARTHandle->USARTx->CR1 &= ~(USART_CR1_M0_Msk |USART_CR1_M1_Msk); // Set word length - 1 Start bit, 8 data bits, n stop bits
    USARTHandle->USARTx->CR1 &= ~(USART_CR1_PCE_Msk); // Disable parity

    USARTHandle->USARTx->CR3 |= USART_CR3_DMAT; // Enable DMA for transmitter

    USARTHandle->USARTx->CR1 |= USART_CR1_TE | USART_CR1_UE; // Start USART transmitter

    // Configure DMA for USART

    // Enable clock for DMA
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

    DMA1_Channel7->CCR &= ~DMA_CCR_EN; // Disable DMA for configuration
    DMA1_Channel7->CCR |= DMA_CCR_DIR | DMA_CCR_MINC; // Direction --> Read from memory, Memory increment mode

    DMA1_CSELR->CSELR &= ~DMA_CSELR_C7S_Msk; // Reset selection
    DMA1_CSELR->CSELR |= (2 << DMA_CSELR_C7S_Pos); // Map to USART2_TX -> see reference manual

    DMA1_Channel7->CPAR = (volatile uint32_t)&USARTHandle->USARTx->TDR; // Set peripheral register to DMA 1

}

uint8_t CheckReady()
{
    if ((DMA1->ISR & DMA_ISR_TCIF7)|| !(DMA1_Channel7->CCR & DMA_CCR_EN))
    {
        return 0;
    }
    return 1;
}


void SendString(char* str)
{
    // Check if USART is ready
    if (CheckReady() == 0)
    {
        const uint32_t str_len = strlen(str);

        DMA1_Channel7->CCR &= ~DMA_CCR_EN; // Disable DMA for configuration
        DMA1->IFCR = DMA_IFCR_CTCIF7; // Delete transfer-complete-flag
        DMA1_Channel7->CMAR = (uint32_t)str; //  Pointer to data buffer
        DMA1_Channel7->CNDTR = str_len;
        DMA1_Channel7->CCR |= DMA_CCR_EN; // Enable DMA

    }

}
;
