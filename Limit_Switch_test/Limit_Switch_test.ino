// GPIOのピン設定
const int switchPin1 = 32; // GPIO25に接続されたスイッチ
const int switchPin2 = 26; // GPIO26に接続されたスイッチ

void setup() {
  Serial.begin(115200); // シリアル通信の開始
  pinMode(switchPin1, INPUT_PULLUP); // GPIO25をプルアップ入力として設定
  pinMode(switchPin2, INPUT_PULLUP); // GPIO26をプルアップ入力として設定
}

void loop() {
  int switchState1 = digitalRead(switchPin1); // GPIO25の状態を読み取り
  int switchState2 = digitalRead(switchPin2); // GPIO26の状態を読み取り

  // スイッチの状態をシリアルモニターに表示
  Serial.print("Switch 1: ");
  if (switchState1 == LOW) {
    Serial.println("Pressed"); // スイッチが押されている場合
  } else {
    Serial.println("Not pressed"); // スイッチが押されていない場合
  }

  Serial.print("Switch 2: ");
  if (switchState2 == LOW) {
    Serial.println("Pressed"); // スイッチが押されている場合
  } else {
    Serial.println("Not pressed"); // スイッチが押されていない場合
  }

  delay(500); // 500ミリ秒待機
}
