#include "qwiic_bt_tiny_config.h"

#define EEPROM_ADR_I2C_ADDRESS  0x01 // Stored I2C Address
#define EEPROM_ADR_BAUD         0x02 // Baud rate index (1-7)

uint8_t loadI2cAddress(void) {
  uint8_t storedI2cAddress = EEPROM.read(EEPROM_ADR_I2C_ADDRESS);
  
  if (storedI2cAddress == 255) {
    storedI2cAddress = I2C_ADDRESS_DEFAULT;
    EEPROM.write(EEPROM_ADR_I2C_ADDRESS, storedI2cAddress);
  }
  
  return storedI2cAddress;
}

void storeI2cAddress(uint8_t address) {
  // Ensure the address is within the allowed range
  if ((address < 0x08) || (address > 0x77)) {
    return;
  }
  EEPROM.write(EEPROM_ADR_I2C_ADDRESS, address);
}

uint8_t loadBaudIndex(void) {
  uint8_t storedBaud = EEPROM.read(EEPROM_ADR_BAUD);
  if (storedBaud == 255) {
    storedBaud = BAUD_9600;
    EEPROM.write(EEPROM_ADR_BAUD, storedBaud);
  }
}

void storeBaudIndex(uint8_t baud) {
  uint8_t baudIndex = baud;
  if ((baudIndex== BAUD_INVALID) || (baudIndex>= NUM_BAUD_RATES)) {
    // Default to 9600 if baud is incorrectly stored
    baudIndex = BAUD_9600;
  }
  EEPROM.write(EEPROM_ADR_BAUD, baudIndex);
}

