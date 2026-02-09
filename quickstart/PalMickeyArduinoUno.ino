/*
 * Pal Mickey IR Blaster - Explorer Edition
 * * Hardware Setup:
 * - Pin 9  -> 220 Ohm Resistor -> IR LED Anode (+)
 * - GND    -> IR LED Cathode (-)
 * * Communication:
 * - Baud Rate: 2400
 * - Expects 8-character Hex strings (e.g., "07010801") followed by a newline (\n)
 */

#include <Arduino.h>

void setup() {
  // Pal Mickey communication must happen at exactly 2400 baud
  Serial.begin(2400);
  while(!Serial); 

  pinMode(9, OUTPUT);

  // Setup Timer 1 to generate a constant 38kHz carrier wave on Pin 9.
  // This is the frequency IR receivers (like Mickey's) are tuned to hear.
  TCCR1A = 0;                      // Clear settings
  TCCR1B = _BV(WGM12) | _BV(CS10); // CTC Mode, No Prescaling
  OCR1A = 210;                     // 16MHz / (2 * 38kHz * 1) - 1 ≈ 210
}

void loop() {
  static char buffer[16];
  static int pos = 0;

  // Listen for incoming Hex strings from the Web Interface
  while (Serial.available() > 0) {
    char ch = Serial.read();
    
    // Process string on newline
    if (ch == '\r' || ch == '\n') {
      if (pos > 0) {
        buffer[pos] = '\0';
        sendPalMickey(buffer);
        pos = 0; 
      }
    } else if (pos < 15) {
      buffer[pos++] = ch;
    }
  }
}

/**
 * Constructs the Pal Mickey IR packet:
 * [Header(2 bytes)] [Command(2 bytes)] [Park/Loc(4 bytes)] [Checksum(1 byte)]
 */
void sendPalMickey(String str) {
  uint8_t packet[9];
  packet[0] = 0x55; // Sync byte 1
  packet[1] = 0xAA; // Sync byte 2
  packet[2] = 0x06; // Length/Type
  packet[3] = 0x01; // Command Type

  // Convert the 8-character hex string into 4 bytes of data
  for(uint8_t i=0; i<4; i++) {
    String hexPart = str.substring(i*2, i*2+2);
    packet[4+i] = (uint8_t) strtol(hexPart.c_str(), NULL, 16);
  }

  // Calculate Checksum (sum of bytes 2 through 7)
  int sum = 0;
  for (int i = 0; i < 6; i++) sum += packet[i + 2];
  packet[8] = (uint8_t)sum;

  // Modulate the 38kHz carrier with our packet data
  for (int i = 0; i < 9; i++) {
    softSerialWrite(packet[i]);
  }
}

/**
 * Mimics a 2400 baud serial signal by turning the 38kHz carrier 
 * ON (for a 0 bit) and OFF (for a 1 bit).
 */
void softSerialWrite(uint8_t b) {
  // 1 second / 2400 bits = 416 microseconds per bit
  const int bitTime = 416; 

  // --- Start Bit (Always Logic LOW / LED ON) ---
  TCCR1A |= _BV(COM1A0); 
  delayMicroseconds(bitTime);

  // --- 8 Data Bits (LSB first) ---
  for (int i = 0; i < 8; i++) {
    if (b & (1 << i)) {
      TCCR1A &= ~_BV(COM1A0); // Logic HIGH = Carrier OFF
    } else {
      TCCR1A |= _BV(COM1A0);  // Logic LOW = Carrier ON
    }
    delayMicroseconds(bitTime);
  }

  // --- Stop Bit (Always Logic HIGH / LED OFF) ---
  TCCR1A &= ~_BV(COM1A0);
  delayMicroseconds(bitTime);
}
