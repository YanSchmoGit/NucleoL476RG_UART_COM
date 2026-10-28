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
    uint8_t bufferOrder[USART_RX_BUFFER_SIZE];
    uint32_t volatile pointerTail;
    uint8_t volatile pointerOrder;
    uint8_t idlePending;
} USARTHandlerReceive_t;

typedef struct
{
    USART_TypeDef* USARTx;
    DMA_TypeDef* DMAx;
    DMA_Channel_TypeDef* DMA_Channel_Tx;
    DMA_Channel_TypeDef* DMA_Channel_Rx;
    DMA_Request_TypeDef* DMA_request;
    uint8_t Interrupt_Tx;
    uint8_t Interrupt_Rx;
    uint8_t DMA_Channel_Tx_Number;
    uint8_t DMA_Channel_Rx_Number;
} USARTMap_t;

typedef struct
{
    GPIO_TypeDef* GPIOPort;
    uint8_t GPIO_Pin_TX;
    uint8_t GPIO_Pin_RX;
    USARTMap_t USARTMap;
    uint32_t BaudRate;
    USARTHandlerSend_t Send;
    USARTHandlerReceive_t Receive;
} USARTHandler_t;

static USARTMap_t USARTMapStorage[] =
{
    {USART1, DMA1, DMA1_Channel4, DMA1_Channel5, DMA1_CSELR, USART1_IRQn, DMA1_Channel5_IRQn, 4, 5},
    {USART2, DMA1, DMA1_Channel6, DMA1_Channel7, DMA1_CSELR, USART2_IRQn, DMA1_Channel7_IRQn, 6, 7},
    {USART3, DMA1, DMA1_Channel2, DMA1_Channel3, DMA1_CSELR, USART3_IRQn, DMA1_Channel3_IRQn, 2, 3},
};


void UART_init(USARTHandler_t* USARTHandle);

uint8_t SendString(char* str, USARTHandler_t* USARTHandle);

uint8_t ReceiveString(char* str, USARTHandler_t* USARTHandler);


#endif //NUCLEO_UART_COM_UART_DRIVER_H
