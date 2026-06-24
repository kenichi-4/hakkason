#ifndef TEMPO_CALCULATOR_H
#define TEMPO_CALCULATOR_H
// ↑ この2行と一番下の #endif はセット。
//   同じヘッダが2回読み込まれても中身が二重に展開されないようにする「おまじない」。
//   （インクルードガードと呼ぶ。全部の .h に付けるもの）

#include <Arduino.h>

// =============================================================
//  TempoCalculator（テンポ計算クラス）
//
//  役割：
//   「振り下ろし（ダウンビート）の間隔[ミリ秒]」を受け取って、
//    ① BPM（1分間の拍数）を計算し、
//    ② それを Slow / Normal / Fast の3段階に分類する。
//
//  入力はどこから来る？
//    DownbeatDetector の getInterval() が「前の振り下ろしから今回までの時間」を
//    ミリ秒で返してくれる。その値を update() に渡すだけ。
//
//  なぜ3段階にする？
//    同期制御Arduinoへは「速い/普通/遅い」の3種類だけ送る約束だから。
//    生のBPM(例:117.3)をそのまま送るより、3段階の方が扱いやすく安定する。
// =============================================================


// テンポの段階を表す名前付きの定数。
// enum（列挙型）＝「決まった選択肢に名前を付ける」しくみ。
// TEMPO_SLOW=0, TEMPO_NORMAL=1, TEMPO_FAST=2 と内部では数字だが、
// 名前で書けるのでコードが読みやすくなる。
enum TempoLevel {
  TEMPO_SLOW,    // 遅い
  TEMPO_NORMAL,  // 普通
  TEMPO_FAST     // 速い
};


class TempoCalculator {
public:
  // コンストラクタ：このクラスを作った瞬間に1回だけ呼ばれる初期化処理。
  TempoCalculator();

  // 【メインの関数】ダウンビート間隔[ミリ秒]を渡す。
  //   戻り値 true  … 有効な間隔で、BPMと段階を更新できた
  //   戻り値 false … 無効だったので何もしなかった
  //                  （最初の振り下ろしで間隔が0、または取りこぼし等で
  //                    ありえない値だった場合）
  // → false の時はテンポを送らない、という判断に使える。
  bool update(unsigned long interval);

  // 今のBPM（小数）を返す。主にデバッグ表示用。
  float getBPM();

  // 今のテンポ段階（SLOW/NORMAL/FAST）を返す。これを送信する。
  TempoLevel getLevel();

  // 直前の update() で段階が「変わった」なら true。
  // 「段階が変わった時だけ送る」ようにしたい時に使える（任意）。
  bool levelChanged();

  // 演奏をやり直す時に、ためてきた値を全部リセットする。
  void reset();

  // ---- 調整用：実測や音側の都合に合わせてあとから変えられる ----

  // 1小節あたりの拍数を設定する。2拍子なら2、4拍子なら4。
  // BPM計算の係数になる（下の .cpp の式を参照）。
  void setBeatsPerMeasure(int value);

  // 段階の境界BPMを設定する。
  //   slowMaxBpm 未満 → SLOW、fastMinBpm 以上 → FAST、その間 → NORMAL
  void setThresholds(float slowMaxBpm, float fastMinBpm);


private:
  // ===== 設定値（調整できる数字） =====
  int   beatsPerMeasure;  // 1小節の拍数。2拍子なら2。
  float slowMaxBpm;       // これ未満を SLOW とする境界
  float fastMinBpm;       // これ以上を FAST とする境界

  // static const = 「変わらない定数」。クラス全体で共通の固定値。
  static const int           SMOOTH_SAMPLES     = 3;     // 中央値をとるために覚えておく本数
  static const unsigned long MIN_VALID_INTERVAL = 200;   // [ms] これより短い間隔は異常として捨てる
  static const unsigned long MAX_VALID_INTERVAL = 2500;  // [ms] これより長い間隔は異常として捨てる
  static const float         HYSTERESIS_BPM;             // 境界に持たせる余白（値は .cpp で定義）

  // ===== 状態（動作中に変化していく変数） =====

  // 直近の間隔を覚えておく配列（リングバッファ）。
  // リングバッファ＝決まった個数の箱を輪のように使い回すしくみ。
  // 新しい値が来たら一番古い箱に上書きする。ここでは最新3個を保持する。
  unsigned long intervals[SMOOTH_SAMPLES];
  int idx;    // 次に書き込む箱の番号（0→1→2→0→…と回る）
  int count;  // 今までにためた個数（最大 SMOOTH_SAMPLES まで増える）

  float      currentBpm;       // 現在のBPM
  TempoLevel currentLevel;     // 現在のテンポ段階
  bool       levelInitialized; // 段階を一度でも確定したか（最初の1回を特別扱いするため）
  bool       changed;          // 直前の update で段階が変わったか

  // ===== 内部だけで使う補助関数 =====
  unsigned long medianInterval();    // ためた間隔の「中央値」を返す
  TempoLevel    classify(float bpm); // BPM から段階を決める（ヒステリシス付き）
};

#endif
