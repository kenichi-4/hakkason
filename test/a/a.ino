#include "b.h"
#include "c.h"
#include "d.h"
#include "e.h"

void setup()
{
  setupMMA8452Q();
  setupPeakDetection();
  setupBPMCalculation();
}

void loop()
{
  float moveAccel = getMoveAcceleration();

  updatePeakAcceleration(moveAccel);
  updateBPMCalculation(moveAccel);
}