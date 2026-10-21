# STM32L476 UART Driver - DMA-Based Transmit and Receive

## Description

The goal of this project is to implement a non-blocking, interrupt- and DMA-driven UART driver on an STM32L476RG
microcontroller (Nucleo board) using a bare-metal, register-level programming approach. The driver establishes a serial
connection between the microcontroller and a PC via the ST-Link virtual COM port and allows sending and receiving
data without occupying the CPU during the transfer.

The peripheral configuration (GPIO, alternate function, USART, DMA) is done through direct register access, avoiding
the vendor HAL to allow fine-grained control over timing and resource usage. All state of a UART instance is kept in a
single handler structure, so the driver code stays free of hard-coded globals as far as possible.

Transmission is based on a ring buffer. `SendString` copies the data into the buffer and returns immediately, so
several strings can be queued back to back without losing any. The DMA works through the buffer block by block, and the
transfer-complete interrupt automatically starts the next block. Reception uses DMA in circular mode, which fills a
receive buffer in the background, while the UART idle-line detection signals the end of a message.

---

## Features

* Register-level configuration of GPIO (AF7), USART and DMA, without HAL
* DMA-based transmission with a ring buffer (`SendString` is non-blocking and queues multiple strings)
* Transfer-complete interrupt starts the next DMA block automatically, including wrap-around at the buffer end
* DMA-based reception in circular mode
* Idle-line detection to recognize the end of a received message
* Overrun error handling, so the receiver does not block
* All instance data bundled in one handler structure (`USARTHandler_t`)

---

## System Architecture

### Component Breakdown
* **Microcontroller:**
  * **STM32L476RG** (ARM Cortex-M4, Nucleo board)
* **Connection:**
  * **ST-Link Virtual COM Port** (USART2, connection to the PC via USB)
  * **PC terminal** (any serial terminal, e.g. 115200 baud, 8N1)
