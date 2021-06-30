#define VREF 320                           // Reference voltage for approx. 5V output

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
  if (vo < VREF) {
    if (OCR1A < (ICR1 - 10))
      OCR1A++;
  }
  else if (vo > VREF) {
    if (OCR1A > 0)
      OCR1A--;
  }
}
