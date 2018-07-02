static uint8_t bytesRequested = 0;

// I2C ring Buffer -- Stores data received from BT module, waiting to be read by I2C master
static char i2cTxBuffer[I2C_TX_BUFFER_SIZE];
static uint8_t i2cBufferTail = 0;
static uint8_t i2cBufferHead = 0;

void initRingBuffer() {
  memset(i2cTxBuffer, 0, I2C_TX_BUFFER_SIZE);
}

static char readRingBufferByte() {
  char c;
  if (ringBufferAvailable()) {
    c = i2cTxBuffer[i2cBufferTail];
    i2cBufferTail = (i2cBufferTail + 1) % I2C_TX_BUFFER_SIZE;
    return c;
  } else {
    return 0;
  }
}

void writeRingBuffer(char c) {
  if (((i2cBufferHead + 1) % I2C_TX_BUFFER_SIZE) == i2cBufferTail) {
    // Overflow
  }
  else {
    i2cTxBuffer[i2cBufferHead] = c;
    i2cBufferHead = (i2cBufferHead + 1) % I2C_TX_BUFFER_SIZE;
  }
}

int ringBufferAvailable(void) {
  if (i2cBufferTail > i2cBufferHead) {
    return (I2C_TX_BUFFER_SIZE - (i2cBufferTail - i2cBufferHead));
  } else {
    return (i2cBufferHead - i2cBufferTail);
  }
}

void i2cRequestAvailable(void) {
  Wire.write(ringBufferAvailable());
}

void setI2cBytesToRead(uint8_t bytesToRead) {
  bytesRequested = bytesToRead;
}

void i2cRequestData(void) {
  for (int i = 0; i < bytesRequested; i++) {
    Wire.write(readRingBufferByte());
  }
  bytesRequested = 0;
}

