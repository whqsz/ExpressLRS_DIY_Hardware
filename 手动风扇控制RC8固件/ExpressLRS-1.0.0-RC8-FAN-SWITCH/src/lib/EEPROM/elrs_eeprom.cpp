#include "elrs_eeprom.h"
#include "targets.h"

#if defined(PLATFORM_STM32)
    #if TARGET_USE_EEPROM && \
            defined(GPIO_PIN_SDA) && (GPIO_PIN_SDA != UNDEF_PIN) && \
            defined(GPIO_PIN_SCL) && (GPIO_PIN_SCL != UNDEF_PIN)
        #if !defined(TARGET_EEPROM_ADDR)
            #define TARGET_EEPROM_ADDR 0x51
            #warning "!! Using default EEPROM address (0x51) !!"
        #endif

        #include <Wire.h>
        #include <extEEPROM.h>
        extEEPROM EEPROM(kbits_2, 1, 1, TARGET_EEPROM_ADDR);
    #else
        #define STM32_USE_FLASH 1
        #include <stm32_eeprom.h>
    #endif
#else
    #include <EEPROM.h>
#endif

void
ELRS_EEPROM::Begin()
{
#if defined(PLATFORM_STM32)
    #if STM32_USE_FLASH
        eeprom_buffer_fill();
    #else // !STM32_USE_FLASH
        /* Initialize I2C */
        Wire.setSDA(GPIO_PIN_SDA);
        Wire.setSCL(GPIO_PIN_SCL);
        Wire.begin();
        /* Initialize EEPROM */
        EEPROM.begin(extEEPROM::twiClock100kHz, &Wire);
    #endif // STM32_USE_FLASH
#else /* !PLATFORM_STM32 */
    EEPROM.begin(RESERVED_EEPROM_SIZE);
#endif /* PLATFORM_STM32 */
}

uint8_t
ELRS_EEPROM::ReadByte(const uint32_t address)
{
    if (address >= RESERVED_EEPROM_SIZE)
    {
        // address is out of bounds
        Serial.println("ERROR! EEPROM address is out of bounds");
        return 0;
    }
#if STM32_USE_FLASH
    return eeprom_buffered_read_byte(address);
#else
    return EEPROM.read(address);
#endif
}

void
ELRS_EEPROM::WriteByte(const uint32_t address, const uint8_t value)
{
    if (address >= RESERVED_EEPROM_SIZE)
    {
        // address is out of bounds
        Serial.println("ERROR! EEPROM address is out of bounds");
        return;
    }
#if STM32_USE_FLASH
    eeprom_buffered_write_byte(address, value);
#else
    EEPROM.write(address, value);
#endif
}

void
ELRS_EEPROM::Commit()
{
#if defined(PLATFORM_ESP32) || defined(PLATFORM_ESP8266)
    if (!EEPROM.commit())
    {
      Serial.println("ERROR! EEPROM commit failed");
    }
#elif STM32_USE_FLASH
    eeprom_buffer_flush();
#endif
}
