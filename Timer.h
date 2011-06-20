#ifndef TIMER_H
#define TIMER_H

#include <time.h>
#include "d3dUtil.h"

class Timer {
public:
  Timer();
  ~Timer();

  void Start();
  void Stop();
  void Stop(WCHAR* output);
private:
  double start, end, time;
};

#endif // TIMER_H