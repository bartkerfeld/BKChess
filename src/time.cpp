#include <sys/time.h>

#include "time.h"

int Time::get_current_time() {
  struct timeval t;
  gettimeofday(&t, 0);
  return t.tv_sec * 1000 + t.tv_usec / 1000;
}
