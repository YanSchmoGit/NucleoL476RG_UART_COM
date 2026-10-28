//
// Created by yan on 10/9/26.
//

#include "../Int/UART_driver.h"

#include "string.h"
#include "stm32l476xx.h"

USARTHandler_t* handlerUSART = NULL;

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

USARTMap_t* AssignUsartDma(USARTHandler_t* USARTHandle)
{
    for (size_t i = 0; i < sizeof(USARTMapStorage); i++)
    {
        if (USARTHandle->USARTMap.USARTx == USARTMapStorage[i].USARTx)
        {
            return &USARTMapStorage[i];
        }
    }

    return NULL;

}

void UART_init(USARTHandler_t* USARTHandle)
{
    // Init GPIO
    InitGPIOPort(USARTHandle->GPIOPort, USARTHandle->GPIO_Pin_TX, USARTHandle->GPIO_Pin_RX);

    // Check channel USART
    USARTHandle->USARTMap = *AssignUsartDma(USARTHandle);

    // Init UART
    if (USARTHandle->USARTMap.USARTx == USART1)
    {
        RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    }
    else if (USARTHandle->USARTMap.USARTx == USART2)
    {
        RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
    }
    else if (USARTHandle->USARTMap.USARTx == USART3)
    {
        RCC->APB1ENR1 |= RCC_APB1ENR1_USART3EN;
    }

    // Configure USART

    // Bauderate - Set bauderate to handler value
    uint32_t systemClock = SystemCoreClock;
    USARTHandle->USARTMap.USARTx->BRR = (uint32_t)systemClock / USARTHandle->BaudRate;

    USARTHandle->USARTMap.USARTx->CR1 &= ~(USART_CR1_M0_Msk | USART_CR1_M1_Msk);
    // Set word length - 1 Start bit, 8 data bits, n stop bits
    USARTHandle->USARTMap.USARTx->CR1 &= ~(USART_CR1_PCE_Msk); // Disable parity

    USARTHandle->USARTMap.USARTx->CR1 |= USART_CR1_IDLEIE; // Enable idle interrupt for receive data

    USARTHandle->USARTMap.USARTx->CR3 |= USART_CR3_DMAT; // Enable DMA for transmitter
    USARTHandle->USARTMap.USARTx->CR3 |= USART_CR3_DMAR; // Enable DMA for receiver

    USARTHandle->USARTMap.USARTx->CR1 |= USART_CR1_UE; //Enable USART
    USARTHandle->USARTMap.USARTx->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE; // Start USART transmitter & receiver

    NVIC_SetPriority(USARTHandle->USARTMap.Interrupt_Rx, 5);
    NVIC_EnableIRQ(USARTHandle->USARTMap.Interrupt_Rx);

    // Configure DMA for USART

    // Enable clock for DMA
    // Check for DMA
    if (USARTHandle->USARTMap.DMAx == DMA1)
    {
        RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
    }
    else if (USARTHandle->USARTMap.DMAx == DMA2)
    {
        RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
    }

    // Configure DMA channel - Transmit
    USARTHandle->USARTMap.DMA_Channel_Rx->CCR &= ~DMA_CCR_EN; // Disable DMA for configuration
    USARTHandle->USARTMap.DMA_Channel_Rx->CCR |= DMA_CCR_DIR | DMA_CCR_MINC; // Direction --> Read from memory, Memory increment mode
    USARTHandle->USARTMap.DMA_Channel_Rx->CCR |= DMA_CCR_TCIE; // Transfer complete interrupt

    USARTHandle->USARTMap.DMA_request->CSELR &=  ~( 0xFUL << ((USARTHandle->USARTMap.DMA_Channel_Rx_Number - 1) * 4));// Reset selection
    USARTHandle->USARTMap.DMA_request->CSELR |= (2 << ((USARTHandle->USARTMap.DMA_Channel_Rx_Number - 1) * 4)); // Map to USARTx_TX -> see reference manual

    USARTHandle->USARTMap.DMA_Channel_Rx->CPAR = (volatile uint32_t)&USARTHandle->USARTMap.USARTx->TDR; // Set peripheral register to DMA

    USARTHandle->Send.pointerHead = 0;
    USARTHandle->Send.pointerTail = 0;
    USARTHandle->Send.lengthDMA = 0;

    handlerUSART = USARTHandle; // Assign handle for interrupt

    NVIC_SetPriority(USARTHandle->USARTMap.Interrupt_Tx, 5);
    NVIC_EnableIRQ(USARTHandle->USARTMap.Interrupt_Tx);

    // Configure DMA channel - Receive
    USARTHandle->USARTMap.DMA_Channel_Tx->CCR &= ~DMA_CCR_EN;
    USARTHandle->USARTMap.DMA_Channel_Tx->CCR |= DMA_CCR_MINC | DMA_CCR_CIRC;

    USARTHandle->USARTMap.DMA_request->CSELR &= ~( 0xFUL << ((USARTHandle->USARTMap.DMA_Channel_Tx_Number - 1) * 4)); // Reset selection
    USARTHandle->USARTMap.DMA_request->CSELR |= (2 << ((USARTHandle->USARTMap.DMA_Channel_Tx_Number - 1) * 4)); // Map to USARTx_RX -> see reference manual
    USARTHandle->USARTMap.DMA_Channel_Tx->CPAR = (uint32_t)&USARTHandle->USARTMap.USARTx->RDR; // Set peripheral register to DMA
    USARTHandle->USARTMap.DMA_Channel_Tx->CMAR = (uint32_t)&USARTHandle->Receive.bufferRx; //  Pointer to data buffer
    USARTHandle->USARTMap.DMA_Channel_Tx->CNDTR = USART_TX_BUFFER_SIZE;
    USARTHandle->USARTMap.DMA_Channel_Tx->CCR |= DMA_CCR_EN;
}

void StartTX(USARTHandler_t* USARTHandle)
{
    // Check if DMA is already running
    if (USARTHandle->Send.lengthDMA != 0 || USARTHandle->Send.pointerHead == USARTHandle->Send.pointerTail)
    {
        return;
    }

    if (USARTHandle->Send.pointerHead >= USARTHandle->Send.pointerTail)
    {
        USARTHandle->Send.lengthDMA = USARTHandle->Send.pointerHead - USARTHandle->Send.pointerTail;
    }
    else
    {
        USARTHandle->Send.lengthDMA = USART_TX_BUFFER_SIZE - USARTHandle->Send.pointerTail;
    }

    USARTHandle->USARTMap.DMA_Channel_Rx->CCR &= ~DMA_CCR_EN; // Disable DMA for configuration
    USARTHandle->USARTMap.DMAx->IFCR = (0x2UL << ((USARTHandle->USARTMap.DMA_Channel_Rx_Number - 1) * 4)); // Delete transfer-complete-flag
    USARTHandle->USARTMap.DMA_Channel_Rx->CMAR = (uint32_t)&USARTHandle->Send.bufferTx[USARTHandle->Send.pointerTail];
    //  Pointer to data buffer
    USARTHandle->USARTMap.DMA_Channel_Rx->CNDTR = (uint32_t)USARTHandle->Send.lengthDMA;
    USARTHandle->USARTMap.DMA_Channel_Rx->CCR |= DMA_CCR_EN; // Enable DMA
}


uint8_t SendString(char* str, USARTHandler_t* USARTHandle)
{
    // Disable interrupt
    NVIC_DisableIRQ(USARTHandle->USARTMap.Interrupt_Tx);

    // Check space in buffer
    const size_t strLen = strlen(str); // Check length of transmit string

    const uint32_t bufferUsed = (USARTHandle->Send.pointerHead - USARTHandle->Send.pointerTail + USART_TX_BUFFER_SIZE) %
        USART_TX_BUFFER_SIZE;
    if (strLen > USART_TX_BUFFER_SIZE - 1 - bufferUsed)
    {
        // Send String is to long for buffer
        NVIC_EnableIRQ(USARTHandle->USARTMap.Interrupt_Tx);
        return 1;
    }

    // Transfer data to buffer

    for (size_t i = 0; i < strLen; i++)
    {
        USARTHandle->Send.bufferTx[USARTHandle->Send.pointerHead] = str[i];
        USARTHandle->Send.pointerHead = (USARTHandle->Send.pointerHead + 1) % USART_TX_BUFFER_SIZE;
    }

    StartTX(USARTHandle);
    NVIC_EnableIRQ(USARTHandle->USARTMap.Interrupt_Tx);
    return 0;
}

uint8_t ReceiveString(char* str, USARTHandler_t* USARTHandle)
{
    // Check for idle flag
    if (USARTHandle->Receive.idlePending == 0)
    {
        // Check for return
        if (USARTHandle->Receive.bufferOrder[USARTHandle->Receive.pointerOrder - 1] == '\r')
        {

            for (size_t i = 0; i < USARTHandle->Receive.pointerOrder - 1; i++)
            {
                str[i] = USARTHandle->Receive.bufferOrder[i];
            }
            USARTHandle->Receive.pointerOrder = 0;
        }

        return 0;
    }

    USARTHandle->Receive.idlePending = 0;
    const uint32_t headPointer = (USART_RX_BUFFER_SIZE - handlerUSART->USARTMap.DMA_Channel_Tx->CNDTR) % USART_RX_BUFFER_SIZE;

    // Get data from receive buffer
    while (USARTHandle->Receive.pointerTail != headPointer)
    {
        USARTHandle->Receive.bufferOrder[USARTHandle->Receive.pointerOrder] = USARTHandle->Receive.bufferRx[USARTHandle->Receive.pointerTail];
        USARTHandle->Receive.pointerOrder++;
        USARTHandle->Receive.pointerTail = (USARTHandle->Receive.pointerTail + 1) % USART_RX_BUFFER_SIZE;
    }

    return 1;
}

void DMA1_Channel7_IRQHandler(void)
{
    if (handlerUSART->USARTMap.DMAx->ISR & (0x2UL << ((handlerUSART->USARTMap.DMA_Channel_Rx_Number - 1) * 4))) // check for DMA_ISR_TCIFx Flag
    {
        handlerUSART->USARTMap.DMAx->IFCR = (0x2UL << ((handlerUSART->USARTMap.DMA_Channel_Rx_Number - 1) * 4));
        handlerUSART->USARTMap.DMA_Channel_Rx->CCR &= ~DMA_CCR_EN;

        handlerUSART->Send.pointerTail = (handlerUSART->Send.pointerTail + handlerUSART->Send.lengthDMA) %
            USART_TX_BUFFER_SIZE;
        handlerUSART->Send.lengthDMA = 0;

        StartTX(handlerUSART);
    }
}

void USART2_IRQHandler(void)
{
    if (handlerUSART->USARTMap.USARTx->ISR & USART_ISR_IDLE)
    {
        handlerUSART->USARTMap.USARTx->ICR |= USART_ICR_IDLECF;
        handlerUSART->Receive.idlePending = 1;
    }

    if (handlerUSART->USARTMap.USARTx->ISR & USART_ISR_ORE)
    {
        handlerUSART->USARTMap.USARTx->ICR |= USART_ICR_ORECF;
    }
}
