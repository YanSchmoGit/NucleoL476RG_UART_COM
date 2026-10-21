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
  };




UART_init(&Com1Handler);

  char receivedData[16] = {0};

  // SendString("TestNeu\r\n");
  SendString("Startup...\r\n", &Com1Handler);
  while (1)
  {

    // Check received data
    ReceiveString(receivedData, &Com1Handler);




    if (receivedData[0] == 'a')
    {
      SendString("a\r\n", &Com1Handler);

      GPIOA->BSRR |= GPIO_BSRR_BS5;
      receivedData[0] = '0';
    }

    if (receivedData[0] == 'b')
    {
      SendString("b\r\n", &Com1Handler);
      GPIOA->BSRR |= GPIO_BSRR_BR5;
      receivedData[0] = '0';
    }

  }

}

