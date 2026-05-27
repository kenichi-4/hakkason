#define BAUD 921600 // ボーレート（速め）
#define PIN 0 // A0 アナログ出⼒
#define RESOLUTION 10 // 量⼦化10bit

#define N 3   // 移動平均の項数

float data[N];   // データ
int i = 0;
float sum = 0;

void setup() {
// マイクのポートを指定
  pinMode(PIN, INPUT);
// シリアル通信の速度を設定(bit per second)
  Serial.begin(BAUD);
// アナログ読み込みの量⼦化精度
  analogReadResolution(RESOLUTION);
}

void loop() {
// A0から読み込み
  int d = analogRead(PIN);
// 読み込んだ値を量⼦化精度で規格化し，電圧を取得
  float a = (float) d / (pow(2, RESOLUTION)-1)*3.3;
  float maxa = 3.3/2.0; // 振幅最⼤値
  a = a - maxa; // 中⼼を0にする

  sum = sum - data[i];
  data[i] = a;
  sum = sum + a;
  i = (i + 1) % N;
  float result = sum / N;

  float mina = -maxa; // 振幅最⼩値
// シリアルモニタに出⼒

  Serial.println(result);


}
