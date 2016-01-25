#include "AutoplanLogging.h"

namespace Logger {
  Options& Options::get()
  {
    static Options INSTANCE;
    return INSTANCE;
  }

  Log& Log::get()
  {
    static Log INSTANCE;
    return INSTANCE;
  }
}
