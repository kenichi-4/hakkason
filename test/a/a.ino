#include "b.h"

void setup()
{
  setupMMA8452Q();
}

void loop()
{
  printPeakAcceleration();
}