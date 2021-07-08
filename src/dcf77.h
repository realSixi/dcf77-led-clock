#include <Arduino.h>

class DCF77Clock {
 public:
  // handle the current dcf77 pulse
  void handlePulse(int pulseLength, int pause);

  // get the current position (if in sync == current second)
  int getPosition();

  // if initial rotation is done
  boolean hasRotated();

  // get the value on the given position
  int getBit(int pos);

  // get the current value of the hour (might be wrong if not fully synced)
  int getHours();
  // get the current value of the minutes (might be wrong if not fully synced)
  int getMinutes();

  // get the current minutes; -1 is returned if not fully synced; might be an
  // old value if parity checks failed
  int getRealMinutes();
  // get the current hours; -1 is returned if not fully synced; might be an old
  // value if parity checks failed
  int getRealHours();

  // calculate the parity for the given range
  int parity(int from, int to);

 private:
  long long unsigned int data = 0LLU;
  byte pos = 0;
  boolean rotated = false;

  int realMinutes = -1;
  int realHours = -1;
};