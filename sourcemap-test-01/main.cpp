#include "main.h"

double meanwrapper(double a, double b) {
  return mean(a, b);
}

// Binding code
EMSCRIPTEN_BINDINGS(Module) {
  emscripten::function("meanwrapper", &meanwrapper);
}
