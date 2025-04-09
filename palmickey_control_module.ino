#include "Arduino.h"

#include <Wire.h>
#include <Adafruit_SSD1306.h>

#include <Encoder.h>

// PIN DIAGRAM
//
// TX -> IR Module RX Pin
// I2C -> OLED Display
// 
//  5-Pin Encoder:
//   
//  PIN 5 ---|    | --- PIN 2
//           |    | --- GND
//   GND  ---|    | --- PIN 3
//
//  NOTE: You must use 10K pullups for PINs 2,3, and 5
//
//  Send Button:
// 
//  PIN 6 ---__---GND
//
//  NOTE: use a 10K pullup for PIN 6

// Program Description
// 
// This program controls an OLED display showing the current park selection in the upper right
// and two location values in the middle of the display (XX and YY).
//
// The encoder dial increases or decreases the XX or YY position value.  The encoder button switches
// which value is being changed.  
//
// By holding the encoder button, the park location value will become underlined allowing you to change it with the
// encoder.
//
// Pressing the Send Button will transmit the IR signal (four two-digit characters; Park and Location)
// to the TX Pin.  This is human-reasable to help with testing and troubleshooting.


// Digital IO used for OLED Display
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)

#define SCREEN_ADDR 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// IO Pins
#define BTN1 5 // Encoder Button
#define BTN2 6 // Send Button

// These pins are special because they make use of an internal timer
#define ENC_A 2
#define ENC_B 3

// Encoder State
Encoder enc(ENC_B,ENC_A);
long enc_pos  = -999;

// State Vars
uint8_t LOCATION = 01;
uint8_t XPOS = 0x01;
uint8_t YPOS = 0x01;
bool SENDING = false;
uint8_t MODE = 01;

uint8_t PUSH_COUNT=0;

void setup() {

  // IR Module
  Serial.begin(2400);
  while(!Serial) {
    ; // wait for serial to begin
  }

  // Encoder
  pinMode(ENC_A, INPUT); // Pulled Up
  pinMode(ENC_B, INPUT); // Pulled Up
  enc.write(XPOS*4);

  // Buttons
  pinMode(BTN1, INPUT); // Pulled Up
  pinMode(BTN2, INPUT); // Pulled Up

  // Initialize the Display
  display.begin(0x00, SCREEN_ADDR);

  display.clearDisplay();
  display.drawPixel(10, 10, SSD1306_WHITE);
  display.display();

  delay(1000);
  updateDisplay();
  delay(1000);
}

void updateDisplay() {
  // Banner
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("My Pal Mickey");
 
  // current park setting
  display.setCursor(110,0);
  if(LOCATION == 1) {
    // Magic Kingdom
    display.println("MK");
  } else if(LOCATION == 2) {
    // Epcot
    display.println("EP");
  } else if(LOCATION == 3) {
    // Hollywood Studios
    display.println("HS");
  } else if(LOCATION == 4) {
    // Animal Kingdom
    display.println("AK");
  }
  
  // Update the X,Y Positions
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5,30);

  if(MODE == 1) {
    // XX Update Mode
    display.drawLine(5,50,45,50,SSD1306_WHITE);

  } else if(MODE == 2) {
    // YY Update Mode
    display.drawLine(65,50,105,50,SSD1306_WHITE);

  } else if(MODE == 3) {
    // Location Update Mode
    display.drawLine(110,8,115,8,SSD1306_WHITE);
  }
  
  char buffer[16];
  sprintf(buffer, "0x%02X,0x%02X", XPOS, YPOS);
  display.println(buffer);

  display.display();
}

void loop() {

  // read encoder position
  long pos = enc.read()/4;
  if(pos != enc_pos){
    enc_pos = pos; // global encoder position
  }

  if(MODE == 1) {
    // XPOS Update
    XPOS = enc_pos;

  } else if(MODE == 2) {
    // YPOS Update
    YPOS = enc_pos;

  } else if(MODE == 3) {
    // Location Update Mode
    LOCATION = (enc_pos % 4) + 1;
  }

  updateDisplay(); // draw the display

  if(digitalRead(BTN1) == LOW) {
    PUSH_COUNT++;

    if(PUSH_COUNT > 3) {
      MODE = 3;
      enc.write(LOCATION);

    } else if(MODE == 1) {
      // XPOS Update
      MODE = 2;
      enc.write(YPOS*4);

    } else if(MODE == 2 || MODE == 3) {
      // YPOS Update
      MODE = 1;
      enc.write(XPOS*4);
    }
    
  }else if(digitalRead(BTN2) == LOW) {
    // Send Button
    sendIRCommand();
  } else {
    PUSH_COUNT = 0;
  }
  
  display.display();
  delay(100);
}


void sendIRCommand() {
    uint8_t data[4];
    data[0] = 0x07; // Default to Florida

    if(LOCATION == 1) {
      // Magic Kingdom
      data[1] = 0x01;
    } else if(LOCATION == 2) {
      // Epcot
      data[1] = 0x02;
    } else if(LOCATION == 3) {
      // Hollywood Studios
      data[1] = 0x03;
    } else if(LOCATION == 4) {
      // Animal Kingdom
      data[1] = 0x04;
    }
    data[2] = XPOS;
    data[3] = YPOS;

    char buf[200];
    sprintf(buf, "%02X%02X%02X%02X", data[0], data[1], data[2], data[3]);
    Serial.println(buf);
    delay(500);
}


