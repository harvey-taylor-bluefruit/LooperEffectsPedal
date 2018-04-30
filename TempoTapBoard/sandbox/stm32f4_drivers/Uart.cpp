// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2017 Absolute Software Ltd t/a Bluefruit (www.bluefruit.co.uk)

#include "stm32f429xx.h"
#include "Uart.h"
#include "Clocks.h"
#include "InterruptPriorities.h"

#include <stddef.h>
#include "SatoruAssert.h"
#include "ByteReceiveInterruptCallbackInterface.h"

namespace stm32_drivers {

namespace
{
   struct UartPeripheral
   {
      USART_TypeDef *Usart;
      volatile uint32_t *PeripheralReset;
      volatile uint32_t *PeripheralClockEnable;
      uint8_t PeripheralIndex;
      IRQn_Type IRQn;
      uint32_t PeripheralClockFrequency;
   };

   const UartPeripheral Usart3Config =
   {
      USART3, &RCC->APB1RSTR, &RCC->APB1ENR, 18, USART3_IRQn, Clocks::PeripheralBusAPB1ClockFrequencyHz
   };

   const UartPeripheral Usart6Config =
   {
      USART6, &RCC->APB2RSTR, &RCC->APB2ENR, 5, USART6_IRQn, Clocks::PeripheralBusAPB2ClockFrequencyHz
   };

   const size_t NumUarts = static_cast<size_t>(Uart::Instance::Count);
   const UartPeripheral *NameToPeripheralMap[NumUarts] 
      = {&Usart3Config, &Usart6Config};

   ByteReceiveInterruptCallbackInterface *aReceiveCallbacks[NumUarts]
      = {nullptr, nullptr};;

   extern "C" void USART3_IRQHandler(void)
   {
      USART_TypeDef *uartBase = Usart3Config.Usart;
      if ((uartBase->CR1 & USART_CR1_RXNEIE) &&
          (uartBase->SR & USART_SR_RXNE)) {
         uint8_t byteReceived = uartBase->DR;
         auto uartIndex = static_cast<size_t>(Uart::Instance::Usart3);
         if (aReceiveCallbacks[uartIndex] != nullptr) {
             aReceiveCallbacks[uartIndex]->OnReceiveInterrupt(byteReceived);
         }
      }
   }

   extern "C" void USART6_IRQHandler(void)
   {
      USART_TypeDef *uartBase = Usart6Config.Usart;
      if ((uartBase->CR1 & USART_CR1_RXNEIE) &&
          (uartBase->SR & USART_SR_RXNE)) {
         uint8_t byteReceived = uartBase->DR;
         auto uartIndex = static_cast<size_t>(Uart::Instance::Usart6);
         if (aReceiveCallbacks[uartIndex] != nullptr) {
             aReceiveCallbacks[uartIndex]->OnReceiveInterrupt(byteReceived);
         }
      }
   }
}

Uart::Uart(Instance instance, uint32_t baudRate) :
   mInstance(instance),
   mBaudRate(baudRate)
{
   mIndex = static_cast<size_t>(instance); 
   switch (mInstance) {
   case Instance::Usart3:
      break;
   case Instance::Usart6:
      break;
   default:
      SatoruAssert("No support for this USART yet");
   };
}

void Uart::Enable()
{
   Clocks::ResetPeripheral(NameToPeripheralMap[mIndex]->PeripheralReset,
                           NameToPeripheralMap[mIndex]->PeripheralIndex);
   Clocks::EnablePeripheralClock(
      NameToPeripheralMap[mIndex]->PeripheralClockEnable,
      NameToPeripheralMap[mIndex]->PeripheralIndex);

   NameToPeripheralMap[mIndex]->Usart->SR &= ~0x3FFU; // clear non-reserved
   NameToPeripheralMap[mIndex]->Usart->CR1 |= USART_CR1_UE; // enable UART

   // keep CR2 reset values for 8 bit, no parity, 1 stop bit

   SetBaudRate();

   // Enable Tx and Rx
   NameToPeripheralMap[mIndex]->Usart->CR1 |= USART_CR1_TE | USART_CR1_RE; 

   // Enable Rx interrupt
   NameToPeripheralMap[mIndex]->Usart->CR1 |= USART_CR1_RXNEIE; 

   NVIC_SetPriority (NameToPeripheralMap[mIndex]->IRQn, InterruptPriorities::Uart);
   NVIC_EnableIRQ(NameToPeripheralMap[mIndex]->IRQn);
}

void Uart::SetReceiveCallback(ByteReceiveInterruptCallbackInterface *callback)
{
   aReceiveCallbacks[mIndex] = callback;
}

void Uart::SetBaudRate() const
{
   // Avoid floating point to increase precision
   uint32_t extendedDivider = 
      (NameToPeripheralMap[mIndex]->PeripheralClockFrequency << 4) / mBaudRate; 
   // Mantissa is 12 bit and is equal to (extendedDivider >> 8)
   SatoruAssert(extendedDivider < ( 1 << 20)); 

   uint32_t mantissa = extendedDivider >> 8;
   uint32_t fraction = (extendedDivider - (mantissa << 8)) >> 4;

   NameToPeripheralMap[mIndex]->Usart->BRR = (mantissa << 4) | fraction;
}

void Uart::SendByteBlocking(uint8_t byte) const
{
   USART_TypeDef *uartBase = NameToPeripheralMap[mIndex]->Usart;
   while (!(uartBase->SR & USART_SR_TXE))
   {
   }
   uartBase->DR = byte;
   while (!(uartBase->SR & USART_SR_TC)) 
   {
   }
}

void Uart::SendStringBlocking(const char *message)
{
   SatoruAssert(message != nullptr);
   while (*message != '\0') {
      SendByteBlocking(*message);
      message++;
   }
}

}
