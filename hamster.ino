#include "pins_arduino.h"

// The pin of the wheel spoke sensor.
// Must be an interrupt pin, on Mega* one of 2, 3, 18, 19, 20, 21.
#define WHEEL_PIN 21
// The interrupt number corresponding to WHEEL_PIN.
// SHOULD be digitalPinToInterrupt(WHEEL_PIN) !
// Due to a bug on my development environment the function is missing, so I put
// an explicit number.
#define WHEEL_INT 2
// The value the sensor reports when a spoke is passing by.
#define WHEEL_PULSE LOW

// Absolute maximum speed of the hamster that we expect to record. This is used
// to filter out any potential jitter from the signal. As far as I know, for
// dwarf hamsters this doesn't exceed 1.
#define MAX_SPEED_M_S 5
// Diameter of the wheel in cm.
#define DIAMETER_CM 22
// The number of spokes the wheel has. This corresponds to the number of
// pulses we receive during one full rotation.
#define SPOKES 2

// How often we want to send the measured values to the serial line.
#define RECORD_EVERY_S 20

typedef unsigned int uint;
typedef unsigned long ulong;

// Derived values:
const float kDistancePerPulseM = DIAMETER_CM / 100.0 * PI / SPOKES;
const float kMinPeriodMs = 1000.0 * kDistancePerPulseM / MAX_SPEED_M_S;

// Holds a sample of measured values.
struct Count {
 public:
  // The duration from which this sample was obtained.
  ulong period_ms;
  // How many spokes have we detected during 'period_ms'.
  uint count;
};

// Records values from the sensor and updates the internal metrics.
class Counter {
 public:
  Counter()
    : start_ms_(millis()),
      counter_(0),
      last_change_ms_(start_ms_),
      last_value_(false) {}

  // Called when the sensor value changes.
  // Argument 'value' is the new value, just after the change.
  uint Record(bool value) volatile {
    const ulong now = millis();
    if (value == last_value_) {
      return counter_;
    }
    if (value) {  // RISING, record, if valid.
      if (now - last_change_ms_ >= kMinPeriodMs) {
	digitalWrite(LED_BUILTIN, HIGH);
	counter_++;
      }
    } else {  // FALLING
      digitalWrite(LED_BUILTIN, LOW);
    }
    last_change_ms_ = now;
    last_value_ = value;
    return counter_;
  }

  // Moves internal metrics into a new 'Count' and resets the internal fields.
  Count MoveOut() volatile {
    Count out;
    noInterrupts();
    const ulong now = millis();
    out.period_ms = now - start_ms_;
    start_ms_ = now;
    out.count = counter_;
    counter_ = 0;
    interrupts();
    return out;
  }

 private:
  ulong start_ms_;
  uint counter_;
  ulong last_change_ms_;
  bool last_value_;
};

volatile Counter counter;

void update_isr() {
  counter.Record(digitalRead(WHEEL_PIN) == WHEEL_PULSE);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  attachInterrupt(WHEEL_INT, update_isr, CHANGE);
}

void loop() {
  Count sample = counter.MoveOut();
  if (sample.period_ms > 0) {
    const float period_s = sample.period_ms / 1000.0;
    const float distance_m = sample.count * kDistancePerPulseM;
    Serial.print("hamster,pin=50 count=");
    Serial.print(sample.count);
    Serial.print(",period_s=");
    Serial.print(period_s, 1);
    Serial.print(",distance_m=");
    Serial.print(distance_m, 2);
    Serial.println();
  }
  delay(/*ms=*/RECORD_EVERY_S * 1000);
}
