#include "MemoryProtectionUnit.h"
#include "SatoruAssert.h"
#include "stm32f7xx.h"

namespace stm32_drivers {

namespace MemoryProtectionUnit {

const uint32_t InstructionFetchedDisabled = 1u << 28u;

const uint32_t TypeExtensionLevel0 = 0u << 19u;
const uint32_t Shareable = 1u << 18u; // can be shared between "agents", e.g. core and DMA controller
const uint32_t Cacheable = 1u << 17u;
const uint32_t Bufferable = 1u << 16u;

const uint32_t FullAccess = 3u << 24u;

const uint32_t MemoryFaultEnabled = 1u << 16u;

namespace Control
{
   const uint32_t Enable = 1u << 0u;
   const uint32_t DefaultMemoryMapForPrivilegedAccess = 1u << 2u;
}

namespace RegionConfig {
   namespace SRAM1 {
      const uint8_t RegionNumber = 0u;
      const uint32_t BaseAddress = 0x20010000;
      const uint32_t Size = 17u; // Region size is 2^(size + 1)
      const uint32_t Flags = InstructionFetchedDisabled | FullAccess |
            TypeExtensionLevel0 | Shareable | Cacheable | Bufferable;
   }

   namespace SRAM2 {
      const uint8_t RegionNumber = 1u;
      const uint32_t BaseAddress = 0x2004C000;
      const uint32_t Size = 13u; // Region size is 2^(size + 1)
      const uint32_t Flags = InstructionFetchedDisabled | FullAccess |
            TypeExtensionLevel0 | Shareable;
   }
}

void Disable()
{
  __DMB();

  SCB->SHCSR &= ~MemoryFaultEnabled;
  MPU->CTRL &= ~Control::Enable;
}

void Enable()
{
   SCB->SHCSR |= MemoryFaultEnabled;
   MPU->CTRL = Control::DefaultMemoryMapForPrivilegedAccess | Control::Enable;

  // Ensure MPU setting take effects
  __DSB();
  __ISB();
}

void ConfigureRegion(uint8_t regionNumber, uint32_t baseAddress, uint32_t size, uint32_t flags)
{
   const uint32_t Enable = 1u << 0u;

   const uint32_t SizePosition = 1u;
   SatoruAssert(size < (1u << 5u));

   MPU->RNR = regionNumber;
   MPU->RBAR = baseAddress;
   MPU->RASR |= (size << SizePosition) |
      flags | Enable;
}

void Configure()
{
   Disable();

   ConfigureRegion(RegionConfig::SRAM1::RegionNumber,
          RegionConfig::SRAM1::BaseAddress,
          RegionConfig::SRAM1::Size,
          RegionConfig::SRAM1::Flags);
   ConfigureRegion(RegionConfig::SRAM2::RegionNumber,
          RegionConfig::SRAM2::BaseAddress,
          RegionConfig::SRAM2::Size,
          RegionConfig::SRAM2::Flags);

   Enable();
}

}
}
