/*
  Qwiic Bluetooth 4.0 - HM-13
  By: Jim Lindblom
  SparkFun Electronics
  Date: July 2, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  An ATtiny84 receives I2C commands from a master and sends/receives commands over serial to the
  HM-13 Bluetooth module. Supported commands include:
  
  0x00: Bytes available - Returns number of bytes stored in ATtiny's I2C receive buffer.
    - Maximum value will be 128 bytes (I2C_TX_BUFFER_SIZE).
    - This is data incoming from the HM-13 module.
  0x01: Read serial data - Reads out a requested number of bytes from the I2C receive buffer.
    - This command requires a second byte, written after the command byte -- NUMBER OF BYTES TO READ
    - [0x02] [NUM_BYTES_TO_READ] [BYTE 0] ... [BYTE N]
    - NUMBER OF BYTES TO READ should not exceed 32 bytes.
  0x02: Write serial data - Write data to the serial device.
    - This commaand requires a second byte, written after the command byte -- NUMBER OF BYTES TO WRITE
    - [0x02] [NUM_BYTES_TO_WRITE] [BYTE 0] ... [BYTE N]
    - NUMBER OF BYTES TO WRITE should not exceed 32 bytes
  0x03: Set baud rate - Sets serial port baud rate to the requested value.
    - This command requires a second byte, written after the command byte -- BAUD_RATE_INDEX.
    - BAUD_RATE_INDEX should be one of the following 8-bit values:
      - 1: 4800 bps             - 5: 57600 bps
      - 2: 9600 bps             - 6: 115200 bps
      - 3: 19200 bps            - 7: 230400 bps
      - 4: 38400 bps
  0x04: Set I2C address - Sets device address of this module
    - This command requires a second byte, written after the command byte -- SET_BAUD_ADDRESS
    - SET_BAUD_ADDRESS should be a value between 0x09 and 0x76
*/

#include <SoftwareSerial.h>
#include <Wire.h>
#include <EEPROM.h>
#include "qwiic_bt_tiny_config.h"

SoftwareSerial btSerial(3, 4); // RX, TX on Tiny

#define I2C_ADDRESS_PIN 1 // MISO on the ATTiny85

// These are the I2C commands supported by this firmware
typedef enum {
  // Read number of bytes available in I2C TX buffer
  I2C_BYTES_AVAILABLE, // 0x00
  // Request a number of bytes from the I2C TX buffer
  I2C_READ_RAW_DATA,   // 0x01
  // Write data to the serial RX buffer
  I2C_WRITE_RAW_DATA,  // 0x02
  // Set serial baud rate
  I2C_SET_BAUD,        // 0x03
  // Set I2C address
  I2C_SET_ADDRESS,     // 0x04
  // Factory reset
  I2C_FACTORY_RESET,   // 0x05

  I2C_INVALID
} qwiic_bt_commands_t;

volatile qwiic_bt_commands_t lastCommand = I2C_INVALID;

typedef enum {
  BAUD_INVALID,  // 0
  BAUD_4800,     // 1
  BAUD_9600,     // 2
  BAUD_19200,    // 3
  BAUD_38400,    // 4
  BAUD_57600,    // 5
  BAUD_115200,   // 6
  BAUD_230400,   // 7
  NUM_BAUD_RATES // 8
} bt_baud_rates_t;
const long btBauds[NUM_BAUD_RATES] = {0, 4800, 9600, 19200, 38400, 57600, 115200, 320400};
bt_baud_rates_t oldBaud;
bt_baud_rates_t newBaud;

void setup() {  
  delay(2000); // Delay to allow BT module to initialize
  
  // Set up software serial port -- Begin by attempting to set baud rate from 115200 to 9600
  btSerial.begin(115200);
  btSerial.print("AT"); // Disconnect if connected
  delay(500);
  btSerial.print("AT+BAUD2"); // Force HM-13 module's UART from 115200 (default) to 9600
  delay(500);
  btSerial.print("AT+RESET"); // Restart module -- should only execute if the module is at 115200 baud
  delay(2000); // Wait for reset if "AT+RESET" worked
  
  setBaud(BAUD_9600);
  btSerial.print("AT"); // Disconnect/test connection
  
  initWire();
}

void loop() {
  if (btSerial.available()) {
    writeRingBuffer(btSerial.read());
  }
  if (oldBaud != newBaud) {
    setBaud(newBaud);
  }
}

void factoryReset(void) {
  // TODO: Reset EEPROM values to defaults.
}

void setBaud(bt_baud_rates_t baud) {
  int baudIndex = baud;
  
  if ((baudIndex == BAUD_INVALID) || (baudIndex>= NUM_BAUD_RATES)) {
    // Default to 9600 if baud is incorrectly stored
    baudIndex = BAUD_9600;
  }
  btSerial.begin(btBauds[baudIndex]);
  storeBaudIndex(baudIndex);
  
  oldBaud = baudIndex;
  newBaud = baudIndex;
}

void i2cReceive(int numBytesReceived) {
  byte commandByte;

  if (numBytesReceived < 1) {
    return; // Error
  }

  // First byte should be a command
  commandByte = Wire.read();

  // Return number of bytes available in Serial RX (I2C TX) buffer
  if ((qwiic_bt_commands_t) commandByte == I2C_BYTES_AVAILABLE) {
    lastCommand = commandByte;
  }
  // Read from Serial RX (I2C TX) buffer
  else if (((qwiic_bt_commands_t) commandByte == I2C_READ_RAW_DATA) && (numBytesReceived == 2)) {
    lastCommand = commandByte;
    setI2cBytesToRead(Wire.read());
  }
  // Send data to the Serial TX buffer
  else if (((qwiic_bt_commands_t) commandByte == I2C_WRITE_RAW_DATA) && (numBytesReceived >= 2)) {
    for (int i = 1; i < numBytesReceived; i++) {
      char c = Wire.read();
      btSerial.write(c);
    }
  }
  // Set software serial baud rate
  else if (((qwiic_bt_commands_t) commandByte == I2C_SET_BAUD) && (numBytesReceived == 2)) {
    newBaud = Wire.read(); // Loop will pick up new baud setting and update software serial
  }
  // Set new I2C address
  else if (((qwiic_bt_commands_t) commandByte == I2C_SET_ADDRESS) && (numBytesReceived == 2)) {
    storeI2cAddress(Wire.read());
  }
  else if (((qwiic_bt_commands_t) commandByte == I2C_FACTORY_RESET) && (numBytesReceived == 2)) {
    uint8_t resetCmd = Wire.read();
    if (resetCmd == 1) {
      factoryReset();
    }
  }
  else {
    lastCommand = I2C_INVALID;
  }
}

void i2cRequest(void) {
  switch (lastCommand) {
    case I2C_BYTES_AVAILABLE:
      i2cRequestAvailable();
      break;
    case I2C_READ_RAW_DATA:
      i2cRequestData();
      break;
    default:
      break;
  }
}

void initWire(void) {
  initRingBuffer();
  Wire.end(); // Before we can change address, need to end wire

  pinMode(I2C_ADDRESS_PIN, INPUT_PULLUP);
  // Read the ADR pin -- if it's low set the I2C address to the pre-configured address
  if (digitalRead(I2C_ADDRESS_PIN) == LOW) {
    Wire.begin(I2C_ADDRESS_JUMPED);
  } else {
    // If the ADR jumper is un-set (pulled high internally), use either the default
    // address or the address configured in EEPROM
    //Wire.begin(loadI2cAddress());
    Wire.begin(I2C_ADDRESS_DEFAULT);
  }

  Wire.onReceive(i2cReceive);
  Wire.onRequest(i2cRequest);
}

