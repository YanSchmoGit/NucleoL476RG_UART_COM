#include "stm32l476xx.h"
#include "../../App/Int/UART_driver.h"

int main(void)
{

  // Blinky, blinky ...
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
  GPIOA->MODER &= ~(GPIO_MODER_MODE5_Msk);
  GPIOA->MODER |= GPIO_MODER_MODE5_0;
  GPIOA->BSRR |= GPIO_BSRR_BR5;

  // USART com
  USARTHandler_t Com1Handler;

  Com1Handler.USARTx = USART2;
  Com1Handler.GPIOPort = GPIOA;
  Com1Handler.GPIO_Pin_RX = 2;
  Com1Handler.GPIO_Pin_TX = 3;
  Com1Handler.BaudRate = 115200;


UART_init(&Com1Handler);

  // SendString("TestNeu\r\n");
  SendString("Null0\r\n");
  while (1)
  {
    //SendString("Test\r\n");
  }

}

