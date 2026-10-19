//
// Created by yan on 10/9/26.
//

#ifndef NUCLEO_UART_COM_UART_DRIVER_H
#define NUCLEO_UART_COM_UART_DRIVER_H
#include "stm32l476xx.h"
#include <string.h>
// USART handle

#define USART_TX_BUFFER_SIZE 256
#define USART_RX_BUFFER_SIZE 256

typedef struct
{
    uint8_t bufferTx[USART_TX_BUFFER_SIZE];
    uint32_t volatile pointerHead;
    uint32_t volatile pointerTail;
    uint32_t volatile lengthDMA;
} USARTHandlerSend_t;

typedef struct
{
    uint8_t bufferRx[USART_RX_BUFFER_SIZE];
};

typedef struct
{
    USART_TypeDef* USARTx;
    GPIO_TypeDef* GPIOPort;
    uint8_t DMA_Channel_Tx;
    uint8_t DMA_Channel_Rx;
    uint8_t GPIO_Pin_TX;
    uint8_t GPIO_Pin_RX;
    uint32_t BaudRate;
    USARTHandlerSend_t Send;
    uint8_t bufferTxPointer;
} USARTHandler_t;

extern char data[20];

void UART_init(USARTHandler_t* USARTHandle);

uint8_t SendString(char* str, USARTHandler_t* USARTHandle);

#endif //NUCLEO_UART_COM_UART_DRIVER_H
