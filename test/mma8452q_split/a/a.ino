#include "data_acquisition.h"
#include "acceleration_processing.h"
#include "peak_detection.h"

void setup()
{
  setupMMA8452Q();
  setupPeakDetection();
}

void loop()
{
  float moveAccel = getMoveAcceleration();
  updatePeakAcceleration(moveAccel);

  delay(50);
}
