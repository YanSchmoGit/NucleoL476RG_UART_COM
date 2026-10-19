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
  USARTHandler_t Com1Handler =
  {
    .USARTx = USART2,
    .GPIOPort = GPIOA,
    .GPIO_Pin_RX = 2,
    .GPIO_Pin_TX = 3,
    .BaudRate = 115200,
    .Send.bufferTx = {0},
  };




UART_init(&Com1Handler);

  // SendString("TestNeu\r\n");
  SendString("Startup...\r\n", &Com1Handler);
  SendString("Eingabe:\r\n", &Com1Handler);
  while (1)
  {
    if (data[0] == 'a')
    {
      RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
      GPIOA->MODER &= ~(GPIO_MODER_MODE5_Msk);
      GPIOA->MODER |= GPIO_MODER_MODE5_0;
      GPIOA->BSRR |= GPIO_BSRR_BS5;
    }


  }

}

