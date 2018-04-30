#include "Uart.h"
#include "Clocks.h"
#include "InterruptPriorities.h"
#include "stm32f7xx.h"
#include "SatoruDebug.h"
#include "SatoruAssert.h"

namespace stm32_drivers {

struct Uart::UartRegisters
{
   USART_TypeDef *Usart;
   volatile uint32_t *PeripheralReset;
   volatile uint32_t *PeripheralClockEnable;
   uint8_t PeripheralIndex;
   IRQn_Type IRQn;
   uint32_t PeripheralClockFrequency;
};

const Uart::UartRegisters Usart6 =
{
   USART6, &RCC->APB2RSTR, &RCC->APB2ENR, 5, USART6_IRQn, Clocks::PeripheralBusAPB2ClockFrequency_Hz
};

const Uart::UartRegisters Usart3 =
{
   USART3, &RCC->APB1RSTR, &RCC->APB1ENR, 18, USART3_IRQn, Clocks::PeripheralBusAPB1ClockFrequency_Hz
};

ByteReceiveInterruptCallbackInterface *receiveCallback = nullptr;

extern "C" void USART6_IRQHandler(void)
{
   USART_TypeDef *uartBase = Usart6.Usart;
   if((uartBase->CR1 & USART_CR1_RXNEIE) &&
      (uartBase->ISR & USART_ISR_RXNE)) {
      uint8_t byteReceived = uartBase->RDR;
      if(receiveCallback != nullptr) {
         receiveCallback->OnReceiveInterrupt(byteReceived);
      }
   }
}

extern "C" void USART3_IRQHandler(void)
{
   USART_TypeDef *uartBase = Usart3.Usart;
   if((uartBase->CR1 & USART_CR1_RXNEIE) &&
      (uartBase->ISR & USART_ISR_RXNE)) {
      uint8_t byteReceived = uartBase->RDR;
      if(receiveCallback != nullptr) {
         receiveCallback->OnReceiveInterrupt(byteReceived);
      }
   }
}

Uart::Uart(Instance instance, uint32_t baudRate) :
   mBaudRate(baudRate)
{
   switch(instance)
   {
   case Instance::Usart3:
      mUartRegisters = &Usart3;
      break;
   case Instance::Usart6:
      mUartRegisters = &Usart6;
      break;
   default:
      FATAL("UART instance not supported!")
   }
}

void Uart::Enable()
{
   Clocks::ResetPeripheral(mUartRegisters->PeripheralReset,
                                  mUartRegisters->PeripheralIndex);
   Clocks::EnablePeripheralClock(mUartRegisters->PeripheralClockEnable,
                                        mUartRegisters->PeripheralIndex);

   mUartRegisters->Usart->CR1 |= USART_CR1_UE; // enable UART
   // keep CR2 reset values for 8 bit, no parity, 1 stop bit

   SetBaudRate();

   mUartRegisters->Usart->CR1 |= USART_CR1_TE | USART_CR1_RE; // enable TX and RX
   mUartRegisters->Usart->CR1 |= USART_CR1_RXNEIE; // enable RX interrupt

   NVIC_SetPriority (mUartRegisters->IRQn, InterruptPriorities::Uart);
   NVIC_EnableIRQ(mUartRegisters->IRQn);
}

void Uart::SetReceiveCallback(ByteReceiveInterruptCallbackInterface *callback)
{
   receiveCallback = callback;
}

void Uart::SetBaudRate() const
{
   uint32_t extendedDivider = (mUartRegisters->PeripheralClockFrequency << 4) / mBaudRate; // avoid using floating point and increase precision
   SatoruAssert(extendedDivider < ( 1 << 20)); // mantissa is 12 bit and is equal to "extendedDivider >> 8"

   uint32_t mantissa = extendedDivider >> 8;
   uint32_t fraction = (extendedDivider - (mantissa << 8)) >> 4;

   mUartRegisters->Usart->BRR = (mantissa << 4) | fraction;
}

void Uart::SendByteBlocking(uint8_t byte) const
{
   USART_TypeDef *uartBase = mUartRegisters->Usart;
   while(!(uartBase->ISR & USART_ISR_TXE)) {}
   uartBase->TDR = byte;
   while(!(uartBase->ISR & USART_ISR_TC)) {}
}

void Uart::SendStringBlocking(const char *message) const
{
   SatoruAssert(message != nullptr);
   while(*message != '\0') {
      SendByteBlocking(*message);
      message++;
   }
}

}
