#include <Arduino.h>
#include <SPI.h>

// ステッピングモーター用のピンの定義
#define PIN_SPI_MOSI 23
#define PIN_SPI_MISO 19
#define PIN_SPI_SCK 18
#define PIN_SPI_SS_A 17
#define PIN_SPI_SS_B 16

// 現在の角度（初期値は0度）
float currentAngle = 0;

void setup() {
  delay(1000);
  Serial.begin(115200);

  // SPI通信のピンの準備
  pinMode(PIN_SPI_MOSI, OUTPUT);
  pinMode(PIN_SPI_MISO, INPUT);
  pinMode(PIN_SPI_SCK, OUTPUT);
  pinMode(PIN_SPI_SS_A, OUTPUT);
  pinMode(PIN_SPI_SS_B, OUTPUT);
  digitalWrite(PIN_SPI_SS_A, HIGH);
  digitalWrite(PIN_SPI_SS_B, HIGH);

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
void L6470_send(unsigned char value, int ssPin) {
  digitalWrite(ssPin, LOW);
  SPI.transfer(value); // 制御信号をSPI通信で送る
  digitalWrite(ssPin, HIGH);
}

//**********************************************
// L6470の初期設定
//**********************************************
void L6470_setup() {
  // デバイス設定
  L6470_send(0x00, PIN_SPI_SS_A);
  L6470_send(0x00, PIN_SPI_SS_B);
  L6470_send(0x00, PIN_SPI_SS_A);
  L6470_send(0x00, PIN_SPI_SS_B);
  L6470_send(0xC0, PIN_SPI_SS_A);

  // 最大回転スピード設定
  L6470_send(0x07, PIN_SPI_SS_A); // レジスタアドレス
  L6470_send(0x20, PIN_SPI_SS_A); // 値 (10bit)

  // その他の設定は必要に応じて追加
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();
    switch (cmd) {
      case 'A':
        // Aが入力された場合、PIN_SPI_SS_Aのみ有効
        digitalWrite(PIN_SPI_SS_B, HIGH);
        digitalWrite(PIN_SPI_SS_A, LOW);
        Serial.println("SS A activated.");
        break;
      case 'B':
        // Bが入力された場合、PIN_SPI_SS_Bのみ有効
        digitalWrite(PIN_SPI_SS_A, HIGH);
        digitalWrite(PIN_SPI_SS_B, LOW);
        Serial.println("SS B activated.");
        break;
      default:
        // それ以外の場合は無視
        Serial.println("Invalid command.");
        break;
    }
  }

  // ここでモータ制御のロジックを実行（省略）
}
