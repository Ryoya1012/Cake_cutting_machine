//2024年11月29日
//Author Wataru ONISHI 
//All rights reserved

#include <Arduino.h>
#include <SPI.h>

// ステッピングモーター用のピンの定義
#define PIN_SPI_MOSI 23
#define PIN_SPI_MISO 19
#define PIN_SPI_SCK 18
#define PIN_SPI_SS 16

// 現在の角度（初期値は0度）
float currentAngle = 0;

void setup() {
  delay(1000);
  Serial.begin(9600);

  // SPI通信のピンの準備
  pinMode(PIN_SPI_MOSI, OUTPUT);
  pinMode(PIN_SPI_MISO, INPUT);
  pinMode(PIN_SPI_SCK, OUTPUT);
  pinMode(PIN_SPI_SS, OUTPUT);
  digitalWrite(PIN_SPI_SS, HIGH);

  // SPI通信の開始設定
  SPI.begin();
  SPI.setDataMode(SPI_MODE3); // SCKの立上りでデータを送受信、アイドル時はHIGH
  SPI.setBitOrder(MSBFIRST); // MSBから送信

  // L6470の初期設定
  L6470_setup();
}

//**********************************************
// SPI通信するための関数
//**********************************************
void L6470_send(unsigned char value) {
  digitalWrite(PIN_SPI_SS, LOW);
  SPI.transfer(value); // 制御信号をSPI通信で送る
  digitalWrite(PIN_SPI_SS, HIGH);
}

//**********************************************
// L6470の初期設定
//**********************************************
void L6470_setup() {
  // デバイス設定
  L6470_send(0x00);
  L6470_send(0x00);
  L6470_send(0x00);
  L6470_send(0x00);
  L6470_send(0xC0);

  // 最大回転スピード設定
  L6470_send(0x07); // レジスタアドレス
  L6470_send(0x20); // 値 (10bit)

  // モータ停止中の電圧設定
  L6470_send(0x09); // レジスタアドレス
  L6470_send(0xFF); // 値 (8bit)

  // モータ定速回転時の電圧設定
  L6470_send(0x0A); // レジスタアドレス
  L6470_send(0xFF); // 値 (8bit)

  // 加速中の電圧設定
  L6470_send(0x0B); // レジスタアドレス
  L6470_send(0xFF); // 値 (8bit)

  // 減速中の電圧設定
  L6470_send(0x0C); // レジスタアドレス
  L6470_send(0xFF); // 値 (8bit)

  // フルステップモードの設定
  L6470_send(0x16); // レジスタアドレス
  L6470_send(0x00); // 値 (8bit)
}

//**********************************************
// 指定角度まで回転させる関数
//**********************************************
void moveToAngle(float targetAngle) {
  // ターゲット角度との差分を計算
  float angleDifference = targetAngle - currentAngle;

  // 必要なステップ数を計算 (1ステップ = 1.8度)
  int steps = angleDifference / 0.6;

  // 回転方向を決定
  bool direction = steps > 0; // 正方向: true, 逆方向: false

  // 絶対値を取得
  steps = abs(steps);

  // モータを回転
  Serial.print("Moving from ");
  Serial.print(currentAngle);
  Serial.print(" degrees to ");
  Serial.print(targetAngle);
  Serial.print(" degrees. Steps: ");
  Serial.println(steps);

  for (int i = 0; i < steps; i++) {
    L6470_send(direction ? 0x51 : 0x50); // 正方向または逆方向
    L6470_send(0x00);                   // スピードデータ上位バイト
    L6470_send(0x20);                   // スピードデータ中位バイト
    L6470_send(0x00);                   // スピードデータ下位バイト
    delay(1);                           // ステップ間の短い遅延（適宜調整）
  }

  // 回転後の角度を更新
  currentAngle = targetAngle;

  // モータ停止
  L6470_send(0xB8); // ハードストップ
  Serial.println("Movement complete.");
}

void loop() {
  // 180度に回転させる
  moveToAngle(180);
  delay(2000);

  // -180度に回転させる
  moveToAngle(-180);
  delay(2000);

}