#ifndef _EMA_H
#define _EMA_H

#include <math.h>

// Computes exponential moving average.
class EMA {
 public:
  // Initializes the class with a 'init' value and a given mean time in ms.
  // This is the time after which the weight of a value drops to 1/2.
  EMA(unsigned long mean_time_ms, double init)
    : factor_((double)mean_time_ms / log(2.0)),
      average_(init),
      last_record_(millis()) {}

  // Records 'value' at time 'time_ms'. This argument will be most likely
  // 'millis()'.
  double Record(unsigned long time_ms, double value) volatile {
    if (!isnan(value) && !isinf(value)) {
      const double alpha = exp(- (double)(time_ms - last_record_) / factor_);
      average_ = alpha * average_ + (1.0 - alpha) * value;
      last_record_ = time_ms;
    }
    return average_;
  }

  // Returns the current value of the average.
  double Average() volatile {
    return average_;
  }

 private:
  const double factor_;
  double average_;
  unsigned long last_record_;
};

#endif  // _EMA_H
