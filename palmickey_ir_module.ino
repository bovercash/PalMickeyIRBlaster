#include <Arduino.h>

// PIN DIAGRAM
//
// RX -> Receive Data
// TX -> IR(-) Cluster
// PIN 9 or 10 -> 47 Ohm Resistor -> IR(+) Cluster
//
// Tested with 3 940nm IR LEDs in series
//

// Program Description
// 
// This program accepts four two-character values interpreted as four HEX values
// used as part of the signal to be transmited.
//
// The signal format is part of the Pal Mickey Sequest below
// 0x55 0xAA 0x06 0x01 SS PP XX YY CHK55
//
// Where SS PP denotes the park and XX YY denotes a location within the park
//
// Example, if the code "07010802" is received, the signal below is transmitted
// 0x55 0xAA 0x06 0x01 0x07 0x01 0x08 0x02 CHK55
//
// The code is sent in plain-text to make is easier for a human to test; this code could be modified to send the value 
// directly if desired.
//
// Pal Mickey will only respond to 2400 Baud transmissions, do not change the baud rate in the code.

void setup() {

  Serial.begin(2400);
  while(!Serial) {
    ; // wait for serial to begin
  }

  // Start setup
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);

  // Clear Timer on Compare Match (CTC) Mode
  bitWrite(TCCR1A, WGM10, 0);
  bitWrite(TCCR1A, WGM11, 0);
  bitWrite(TCCR1B, WGM12, 1);
  bitWrite(TCCR1B, WGM13, 0);

  // Toggle OC1A and OC1B on Compare Match.
  bitWrite(TCCR1A, COM1A0, 1);
  bitWrite(TCCR1A, COM1A1, 0);
  bitWrite(TCCR1A, COM1B0, 1);
  bitWrite(TCCR1A, COM1B1, 0);

  // No prescaling
  bitWrite(TCCR1B, CS10, 1);
  bitWrite(TCCR1B, CS11, 0);
  bitWrite(TCCR1B, CS12, 0);

  OCR1A = 210; // 16MHz CPU clock/210 = 76190 cycles/sec = 2*38kHz.
  OCR1B = 210; // Need both on/off for a proper cycle, so 2*desired rate
}

byte buf[100];
const int bufsize = 254;
char dline[bufsize];
boolean done = false;
int p = 0;

void loop() {
  char ch;
  while (Serial.available() > 0 && p < bufsize && done == false) {
    ch = Serial.read();
    if (ch >= '0') {
      dline[p] = ch;
      p++;
    }
    if (ch == '\r' || ch == '\n' ) {
      done = true;
    }
  }
  if (done) {
    dline[p] = '\0';
    uint8_t *data = generateSequence(dline);
    Serial.write(data, 9); // This transmits the IR signal
    delay(200);
    done = false;
    p = 0;
  }
}

uint8_t* generateSequence(String str) {
  //55 AA 06 01 SS PP XX YY CHK
  uint8_t data[9];
  data[0] = 0x55;
  data[1] = 0xAA;
  data[2] = 0x06;
  data[3] = 0x01;

  for(uint8_t i=0; i<4; i++) {
    // expected string is four two-digit hex codes
    char hexStr[2];
    hexStr[0] = str[i*2];
    hexStr[1] = str[i*2+1];
    data[4+i] = intfromhex(hexStr);
  }

  // checksum
  data[8] = calc55Chk(data);
  memcpy(buf, data, sizeof(data));
  return buf;
}

uint8_t intfromhex(char hexstr[2]){
  byte val = 0;
  hexstr[0] -= '0'; hexstr[1] -= '0';
  if (hexstr[0] < 10) {
    val = hexstr[0] << 4;
  } else if (hexstr[0] > 16 && hexstr[0] < 23) {
    val = (hexstr[0] - 7) << 4;
  } else if (hexstr[0] > 48 && hexstr[0] < 55) {
    val = (hexstr[0] - 39) << 4;
  } else {
    return 256;//invalid code
  }
  if (hexstr[1] < 10) {
    val += hexstr[1];
  } else if (hexstr[1] > 16 && hexstr[1] < 23) {
    val += hexstr[1] - 7;
  } else if (hexstr[1] > 48 && hexstr[1] < 55) {
    val += hexstr[1] - 39;
  } else {
    return 256;//invalid code
  }
  return (uint8_t)val;
}

byte calc55Chk(byte *data) {
  int i;
  //4 bit val of data[2] to get length
  int len = data[2] & B00001111;
  int sum = 0; //sum likely to exceed single byte capacity
  //skip the 55 AA and simply add up the rest
  for (i = 0; i < len; i++) {
    sum += data[i + 2];
  }
  return (byte) sum;
}

