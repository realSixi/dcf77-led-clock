#include "dcf77.h"

void DCF77Clock::handlePulse(int pulseLength, int pause) {
  if (pulseLength < 150) {
    data &= ~(1LLU << pos);
  } else if (pulseLength < 250) {
    data |= 1LLU << pos;
  }
  pos = (pos + 1) % 60;

  if (pause > 1200) {
    pos = (pos + 1) % 60;

    data = (data >> pos) | (data << (60 - pos));
    Serial.printf("rotate %d\n", pos);
    pos = 0;
    rotated = true;

    if (this->getBit(20) == 1) {  // check if start of time is set correctly
      if (this->getBit(58) == this->parity(36, 57)) {    // parity for date
        if (this->getBit(28) == this->parity(21, 27)) {  // parity for minute
          realMinutes = getMinutes();
        }
        if (this->getBit(35) == this->parity(29, 34)) {  // parity for hour
          realHours = getHours();
        }
      }
    }
  }
};

int DCF77Clock::getPosition() { return (pos + 59) % 60; }

boolean DCF77Clock::hasRotated() { return rotated; };

int DCF77Clock::getBit(int pos) { return bitRead(data, pos); }

int DCF77Clock::getHours() {
  uint8 hourTens = (data >> 33) & 0b11;
  uint8 hourOne = (data >> 29) & 0b1111;
  return 10 * hourTens + hourOne;
}

int DCF77Clock::getMinutes() {
  uint8 minTens = (data >> 25) & 0b111;
  uint8 minOne = (data >> 21) & 0b1111;
  return 10 * minTens + minOne;
}

int DCF77Clock::getRealMinutes() { return realMinutes; }
int DCF77Clock::getRealHours() { return realHours; }

int DCF77Clock::parity(int from, int to) {
  int result = 0;
  for (int i = from; i <= to; i++) {
    result ^= ((this->data >> i) & 1);
  }
  return result;
}
