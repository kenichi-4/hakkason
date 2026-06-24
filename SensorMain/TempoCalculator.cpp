#include "TempoCalculator.h"

// HYSTERESIS_BPM の中身（値）をここで決める。
// 「段階の境界に持たせる余白[BPM]」のこと。
//
//  なぜ必要？
//   もしBPMが境界(80)のすぐ上をフラフラすると、80.1→79.9→80.2…のたびに
//   SLOW⇄NORMAL が切り替わって、音がガクガクしてしまう。
//   そこで「一度SLOWになったら、80+5=85 を超えるまではNORMALに上げない」
//   というように境界に“のりしろ”を作る。これをヒステリシスと呼ぶ。
//   （ドアの建て付けに少し遊びを持たせて、ガタガタ鳴らないようにするイメージ）
const float TempoCalculator::HYSTERESIS_BPM = 5.0;


// ---------------------------------------------------------------
//  コンストラクタ：オブジェクトを作った時に1回だけ走る初期化。
//  すべての変数に最初の値を入れておく（入れ忘れるとゴミ値で誤動作する）。
// ---------------------------------------------------------------
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


// ---------------------------------------------------------------
//  update：ダウンビートが来るたびに呼ぶメイン処理。
//  「間隔[ms]」を受け取って、BPMと段階を更新する。
// ---------------------------------------------------------------
bool TempoCalculator::update(unsigned long interval) {
  changed = false;  // 今回の呼び出しでは、まだ段階は変わっていない

  // --- ステップ1：おかしな間隔をはじく（妥当性チェック）---
  // ・最初の振り下ろしは前の拍がないので interval == 0 で渡ってくる
  // ・拍を取りこぼしたり、途中で止めたりすると極端な値になる
  // こういう値を計算に入れると BPM が暴れるので、ここで捨てて終了。
  if (interval < MIN_VALID_INTERVAL || interval > MAX_VALID_INTERVAL) {
    return false;
  }

  // --- ステップ2：間隔をリングバッファに保存する ---
  intervals[idx] = interval;              // 今の箱に書き込む
  idx = (idx + 1) % SMOOTH_SAMPLES;       // 次の箱へ。3まで行ったら0に戻る（% は余り）
  if (count < SMOOTH_SAMPLES) {
    count++;                               // ためた個数を増やす（最大3で止まる）
  }

  // --- ステップ3：中央値で平滑化する ---
  // 平均ではなく「中央値」を使うのがポイント。
  // 例）拍を1回取りこぼすと、その間隔だけ約2倍(500→1000)になる。
  //   ・平均  [500,1000,500] → 666  … 引きずられて誤判定
  //   ・中央値[500,1000,500] → 500  … 外れ値を無視できる
  unsigned long smoothed = medianInterval();

  // --- ステップ4：間隔[ms]からBPMを計算する ---
  // BPM = 1小節の拍数 × 60000[ms] ÷ 1小節の時間[ms]
  //
  // 例）2拍子で、振り下ろしの間隔が1000ms(=1秒)だったとき：
  //     BPM = 2 × 60000 ÷ 1000 = 120
  //   → 振り下ろしが速くなる(間隔が短くなる)ほどBPMは大きくなる。
  currentBpm = (float)beatsPerMeasure * 60000.0 / (float)smoothed;

  // --- ステップ5：BPMから段階を決める（ヒステリシス付き）---
  TempoLevel newLevel = classify(currentBpm);

  if (!levelInitialized) {
    // 一番最初の確定：まだ段階を持っていないので、そのまま採用する
    currentLevel     = newLevel;
    levelInitialized = true;
    changed          = true;   // 最初の確定も「変化した」とみなす
  } else if (newLevel != currentLevel) {
    // 2回目以降：前回と違う段階になったときだけ更新する
    currentLevel = newLevel;
    changed      = true;
  }

  return true;  // ちゃんと更新できた
}


// ---------------------------------------------------------------
//  medianInterval：ためた間隔の「中央値」を返す。
//  中央値＝小さい順に並べたとき、ちょうど真ん中に来る値。
// ---------------------------------------------------------------
unsigned long TempoCalculator::medianInterval() {
  if (count == 0) {
    return 0;  // まだ何もたまっていない（通常ここには来ない）
  }

  // ためている分だけ別の配列にコピーする。
  // （元の intervals を並べ替えるとリングバッファが壊れるのでコピーを使う）
  unsigned long tmp[SMOOTH_SAMPLES];
  for (int i = 0; i < count; i++) {
    tmp[i] = intervals[i];
  }

  // 小さい順に並べ替える（挿入ソート）。
  // 要素は最大3個なので、難しいソートは不要。
  // やっていること：左から見て、注目の値を「自分より大きいものを右にずらして」
  //   正しい位置に差し込む、を繰り返す。
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


// ---------------------------------------------------------------
//  classify：BPMを受け取って SLOW / NORMAL / FAST のどれかを返す。
//  境界にヒステリシス（余白）を持たせて、ばたつきを防ぐ。
// ---------------------------------------------------------------
TempoLevel TempoCalculator::classify(float bpm) {
  // 一番最初だけは、余白なしの素直な境界で段階を決める。
  if (!levelInitialized) {
    if (bpm < slowMaxBpm)  return TEMPO_SLOW;
    if (bpm >= fastMinBpm) return TEMPO_FAST;
    return TEMPO_NORMAL;
  }

  // 2回目以降は「今いる段階」に応じて境界を少しずらす＝ヒステリシス。
  //
  //   下の境界(SLOWとNORMALの境目, 名目80):
  //     今SLOWなら 80+5=85 … 85を超えないとNORMALに上がれない
  //     今NORMAL以上なら 80-5=75 … 75を下回らないとSLOWに落ちない
  //
  //   上の境界(NORMALとFASTの境目, 名目130):
  //     今FASTなら 130-5=125 … 125を下回らないとNORMALに落ちない
  //     今NORMAL以下なら 130+5=135 … 135以上にならないとFASTに上がれない
  //
  // こうすると、一度決まった段階は「はっきり変わった時」しか切り替わらない。
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


// ---------------------------------------------------------------
//  以下はシンプルな取り出し・設定用の関数（ゲッター/セッター）
// ---------------------------------------------------------------

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
