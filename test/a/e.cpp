#include "e.h"

// 拍として判定する加速度のしきい値
const float BEAT_THRESHOLD = 0.35;

// 短時間の連続検出を防ぐ時間
const unsigned long DEBOUNCE_TIME = 300;

// BPMを平均する個数
const int BPM_BUFFER_SIZE = 4;

unsigned long lastBeatTime = 0;
bool wasOverThreshold = false;

float bpmBuffer[BPM_BUFFER_SIZE];
int bpmIndex = 0;
int bpmCount = 0;

float currentBPM = 0.0;

void setupBPMCalculation()
{
  lastBeatTime = 0;
  wasOverThreshold = false;

  bpmIndex = 0;
  bpmCount = 0;
  currentBPM = 0.0;

  for (int i = 0; i < BPM_BUFFER_SIZE; i++)
  {
    bpmBuffer[i] = 0.0;
  }
}

void updateBPMCalculation(float moveAccel)
{
  unsigned long currentTime = millis();

  // 加速度がしきい値を超えたか
  bool isOverThreshold = moveAccel > BEAT_THRESHOLD;

  // しきい値を下から上に超えた瞬間だけ拍として扱う
  if (isOverThreshold && !wasOverThreshold)
  {
    // デバウンス処理
    if (lastBeatTime == 0 || currentTime - lastBeatTime >= DEBOUNCE_TIME)
    {
      if (lastBeatTime != 0)
      {
        unsigned long interval = currentTime - lastBeatTime;

        // BPM = 60000 / 拍間隔[ms]
        float instantBPM = 60000.0 / interval;

        // 異常値を除外
        if (instantBPM >= 40.0 && instantBPM <= 240.0)
        {
          bpmBuffer[bpmIndex] = instantBPM;
          bpmIndex = (bpmIndex + 1) % BPM_BUFFER_SIZE;

          if (bpmCount < BPM_BUFFER_SIZE)
          {
            bpmCount++;
          }

          float sum = 0.0;
          for (int i = 0; i < bpmCount; i++)
          {
            sum += bpmBuffer[i];
          }

          currentBPM = sum / bpmCount;

          Serial.print("BPM: ");
          Serial.print(currentBPM, 1);

          Serial.print("  Level: ");
          Serial.println(getTempoLevel());
        }
      }

      lastBeatTime = currentTime;
    }
  }

  wasOverThreshold = isOverThreshold;
}

float getCurrentBPM()
{
  return currentBPM;
}

int getTempoLevel()
{
  if (currentBPM == 0.0)
  {
    return 0; // 未計測
  }
  else if (currentBPM < 80.0)
  {
    return 1; // Slow
  }
  else if (currentBPM < 130.0)
  {
    return 2; // Normal
  }
  else
  {
    return 3; // Fast
  }
}