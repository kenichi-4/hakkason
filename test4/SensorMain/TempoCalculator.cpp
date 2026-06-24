#include "TempoCalculator.h"

const float TempoCalculator::HYSTERESIS_BPM = 5.0;


TempoCalculator::TempoCalculator() {
  beatsPerMeasure = 2;      // 2拍子（音側の拍子に合わせて変更可）
  slowMaxBpm      = 100.0;   // 80未満をSLOW（実測で調整）
  fastMinBpm      = 160.0;  // 130以上をFAST（実測で調整）

  idx   = 0;
  count = 0;
  for (int i = 0; i < SMOOTH_SAMPLES; i++) {
    intervals[i] = 0;       // 間隔の箱を全部0で初期化
  }

  currentBpm       = 0.0;
  currentLevel     = TEMPO_NORMAL;
  levelInitialized = false; // まだ一度も段階を決めていない
  changed          = false;
}


bool TempoCalculator::update(unsigned long interval) {
  changed = false;  

  if (interval < MIN_VALID_INTERVAL || interval > MAX_VALID_INTERVAL) {
    return false;
  }

  intervals[idx] = interval;              // 今の箱に書き込む
  idx = (idx + 1) % SMOOTH_SAMPLES;       // 次の箱へ。3まで行ったら0に戻る（% は余り）
  if (count < SMOOTH_SAMPLES) {
    count++;                               // ためた個数を増やす（最大3で止まる）
  }

  unsigned long smoothed = medianInterval();


  currentBpm = (float)beatsPerMeasure * 60000.0 / (float)smoothed;

  TempoLevel newLevel = classify(currentBpm);

  if (!levelInitialized) {
    currentLevel     = newLevel;
    levelInitialized = true;
    changed          = true;   
  } else if (newLevel != currentLevel) {
    currentLevel = newLevel;
    changed      = true;
  }

  return true;  
}



unsigned long TempoCalculator::medianInterval() {
  if (count == 0) {
    return 0;  
  }


  unsigned long tmp[SMOOTH_SAMPLES];
  for (int i = 0; i < count; i++) {
    tmp[i] = intervals[i];
  }


  for (int i = 1; i < count; i++) {
    unsigned long key = tmp[i];   // 差し込みたい値
    int j = i - 1;
    while (j >= 0 && tmp[j] > key) {
      tmp[j + 1] = tmp[j];        // 大きい値を1つ右へずらす
      j--;
    }
    tmp[j + 1] = key;             // 空いた場所に差し込む
  }

  // 並べ替えたあと、真ん中の値を返す。
  if (count % 2 == 1) {
    return tmp[count / 2];                            // 個数が奇数：ど真ん中
  }
  return (tmp[count / 2 - 1] + tmp[count / 2]) / 2;   // 個数が偶数：中央2つの平均
}



TempoLevel TempoCalculator::classify(float bpm) {
  // 一番最初だけは、余白なしの素直な境界で段階を決める。
  if (!levelInitialized) {
    if (bpm < slowMaxBpm)  return TEMPO_SLOW;
    if (bpm >= fastMinBpm) return TEMPO_FAST;
    return TEMPO_NORMAL;
  }


  float lower = (currentLevel == TEMPO_SLOW)
              ? slowMaxBpm + HYSTERESIS_BPM
              : slowMaxBpm - HYSTERESIS_BPM;
  float upper = (currentLevel == TEMPO_FAST)
              ? fastMinBpm - HYSTERESIS_BPM
              : fastMinBpm + HYSTERESIS_BPM;

  if (bpm < lower)  return TEMPO_SLOW;
  if (bpm >= upper) return TEMPO_FAST;
  return TEMPO_NORMAL;
}


// 現在のBPMを返す
float TempoCalculator::getBPM() {
  return currentBpm;
}

// 現在のテンポ段階を返す
TempoLevel TempoCalculator::getLevel() {
  return currentLevel;
}

// 直前の update で段階が変わったかを返す
bool TempoCalculator::levelChanged() {
  return changed;
}

// 状態を全部リセットする（演奏をやり直す時に呼ぶ）
void TempoCalculator::reset() {
  idx   = 0;
  count = 0;
  for (int i = 0; i < SMOOTH_SAMPLES; i++) {
    intervals[i] = 0;
  }
  currentBpm       = 0.0;
  currentLevel     = TEMPO_NORMAL;
  levelInitialized = false;
  changed          = false;
}

// 1小節あたりの拍数を設定する（2拍子なら2）
void TempoCalculator::setBeatsPerMeasure(int value) {
  if (value > 0) {           // 0や負の数を入れると0割りになるので弾く
    beatsPerMeasure = value;
  }
}

// 段階の境界BPMを設定する
void TempoCalculator::setThresholds(float slowMax, float fastMin) {
  slowMaxBpm = slowMax;
  fastMinBpm = fastMin;
}
