/*
 * Copyright (c) 2021 Sayantan Sinha
 *
 * MIT License
 */

#define VREF 320                           // Reference voltage for approx. 5V output
#define DUTY_MAX (ICR1 - 10)
#define H(X) (X / 2)                       // A gain factor in the proportional control action

const int pinFb = A0;                      // Output voltage feedback at pin A0

void setup()
{
  DDRB |= 1 << PB1;                         // Set D9 as output
  TCCR1A = 0b10000010;                      // Pin D9 (OCR1A): Clear on compare match
  TCCR1B = 0b00011001;                      // Fast PWM (Mode 14); Clock Select = System clock (No Prescaling) [Ref. Data Sheet, p. 172]
  ICR1 = 399;                               // f_sw = 16 MHz / (ICR1 + 1) = 40 kHz
  OCR1A = 0;
}

void loop()
{
  unsigned int vo = analogRead(pinFb);
  int e = VREF - vo;                          // Error
  if (e > 0) {                                // Output is less than reference voltage
    if ((OCR1A + H(e)) < DUTY_MAX) {
      OCR1A += H(e);
    }
    else
      OCR1A = DUTY_MAX;
  }
  else if (e < 0) {                           // Output is higher than the reference voltage
    if (OCR1A > -H(e)) {
      OCR1A += H(e);
    }
    else {
      OCR1A = 0;
    }
  }
}
