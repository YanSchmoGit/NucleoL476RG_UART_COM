//
// Created by yan on 10/9/26.
//

#ifndef NUCLEO_UART_COM_UART_DRIVER_H
#define NUCLEO_UART_COM_UART_DRIVER_H
#include "stm32l476xx.h"

// USART handle

typedef struct
{
    USART_TypeDef* USARTx;
    GPIO_TypeDef* GPIOPort;
    uint8_t GPIO_Pin_TX;
    uint8_t GPIO_Pin_RX;
    uint32_t BaudRate;
} USARTHandler_t;

void UART_init(const USARTHandler_t* USARTHandle);

void SendString(char* str);

#endif //NUCLEO_UART_COM_UART_DRIVER_H
