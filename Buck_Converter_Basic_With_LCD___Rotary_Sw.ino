/*
 * Copyright (c) 2021 Sayantan Sinha
 *
 * MIT License
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DUTY_MAX (ICR1 - 10)
#define H(X) (X / 2)                       // A gain factor in the proportional control action
#define CLK 0
#define DAT 4
#define SW 5
#define DEBOUNCE_TIME 30
#define I2C_LCD 0x3f                       // Address of the I2C LCD backpack module

const int pinFb = A0;                      // Output voltage feedback at pin A0
const float fact = 0.015625;               // An emperical factor to convert analog f/b reading to voltage
const unsigned int vrefMax = 640;          // Analog read equivalent of desired maximum output voltage
unsigned int vref = 320;                   // Analog read equivalent of desired output voltage
unsigned int voPrev = 0;
unsigned int vrefBuff = vref;
unsigned int vrefBuffPrev = 0;
unsigned long int senTime = 0;
unsigned long int swTime = 0;
unsigned long int dispTime = 0;
bool dtState = 1;
bool clkState = 1;

LiquidCrystal_I2C lcd(I2C_LCD, 16, 2);

void setup()
{
  DDRB |= 1 << PB1;                                               // Set D9 as output
  DDRB &= ~((1 << DAT) | (1 << CLK) | (1 << SW));                 // Data (D12), Clock (D8), & Switch (D13) pins as input
  TCCR1A = 0b10000010;                                            // Pin D9 (OCR1A): Clear on compare match
  TCCR1B = 0b00011001;                                            // Fast PWM (Mode 14); Clock Select = System clock (No Prescaling) [Ref. Data Sheet, p. 172]
  ICR1 = 399;                                                     // f_sw = 16 MHz / (ICR1 + 1) = 40 kHz
  OCR1A = 0;                                                      // Duty cycle = (OCR1A / ICR1) * 100%
  
  lcd.begin();
  lcd.print("V_OUT ");
  lcd.print((float)analogRead(pinFb) * fact);
  lcd.print(" V");
  lcd.setCursor(0, 1);
  lcd.print("V_SET ");
  lcd.print((float)vref * fact);
  lcd.print(" V");
}

void loop()
{
  unsigned int vo = analogRead(pinFb);
  int e = vref - vo;                                                   // Error
  if (e > 0) {                                                         // Output is less than reference voltage
    if ((OCR1A + H(e)) < DUTY_MAX) {
      OCR1A += H(e);
    }
    else
      OCR1A = DUTY_MAX;
  }
  else if (e < 0) {                                                    // Output is higher than the reference voltage
    if (OCR1A > -H(e)) {
      OCR1A += H(e);
    }
    else {
      OCR1A = 0;
    }
  }
  if ((PINB & (1 << CLK)) != clkState) {                               // Singnal at clock pin has been changed
    if (clkState) {                                                    // If clock was at high prior to the change
      clkState = 0;
      if ((millis() - senTime) > DEBOUNCE_TIME) {                      // If the previous rotary read time was more than DEBOUNCE_TIME earlier
        if (PINB & (1 << DAT))                                         // Clockwise rotation
          vrefBuff = vrefBuff < vrefMax ? vrefBuff + 1 : vrefMax;      // Increase set voltage
        else                                                           // Counter clockwise rotation
          vrefBuff = vrefBuff > 0 ? vrefBuff - 1 : vrefBuff;           // Decrease set voltage
        senTime = millis();
      }
    }
    else
      clkState = 1;
  }
  if (!(PINB & (1 << SW))) {                                           // Sw has been pressed
    if (!swTime)
      swTime = millis();
    else if ((millis() - swTime) < DEBOUNCE_TIME);
    else {
      vref = vrefBuff;
    }
  }
  else {
    swTime = 0;
  }
  if ((millis() - dispTime) > 800) {                                   // Update display in 800 ms interval
    if (vo != voPrev) {
      lcd.setCursor(6, 0);
      lcd.print((float)vo * fact);
      voPrev = vo;
    }
    if (vrefBuff != vrefBuffPrev) {
      lcd.setCursor(6, 1);
      lcd.print((float)vrefBuff * fact);
      vrefBuffPrev = vrefBuff;
    }
    dispTime = millis();
  }
}
