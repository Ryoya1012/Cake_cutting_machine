#include <Arduino.h>
#include <SPI.h>

// ピン定義
#define PWM_A_PIN 14           // モータードライバのPWM Aピン
#define PWM_B_PIN 27           // モータードライバのPWM Bピン
#define LIMIT_SWITCH_A 32      // リミットスイッチAのピン
#define LIMIT_SWITCH_B 26      // リミットスイッチBのピン
#define PIN_SPI_MOSI 23        // ステッピングモータ用 SPI MOSI
#define PIN_SPI_MISO 19        // ステッピングモータ用 SPI MISO
#define PIN_SPI_SCK 18         // ステッピングモータ用 SPI SCK
#define PIN_SPI_SS_A 17        // ステッピングモータA用 SPI SS
#define PIN_SPI_SS_B 16        // ステッピングモータB用 SPI SS

// PWM設定
#define PWM_CHANNEL_A 0        // PWMチャンネルA
#define PWM_CHANNEL_B 1        // PWMチャンネルB
#define PWM_FREQUENCY 5000     // 5kHz
#define PWM_RESOLUTION 8       // 8ビット
#define PWM_DUTY_CYCLE 100     // PWMのデューティサイクル (0-255)

// ステッピングモータ設定
#define STEP_ANGLE 0.45        // 1ステップ当たりの角度（度）

// グローバル変数
bool motor_running = false;    // モーターが動作中かどうか
bool direction = true;         // true: 正転, false: 逆転
bool switch_A_enabled = true;  // スイッチAが有効かどうか
bool switch_B_enabled = true;  // スイッチBが有効かどうか
String str;                    // シリアルデータの文字列
float target_angle;            // ステッピングモータの目標角度
float currentAngle_A = 0;      // ステッピングモータAの現在角度
float currentAngle_B = 0;      // ステッピングモータBの現在角度

void setup() {
  // シリアル通信初期化
  Serial.begin(115200);
  Serial.println("System Initialized");

  // モータードライバPWMの初期化
  ledcSetup(PWM_CHANNEL_A, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_B, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcAttachPin(PWM_A_PIN, PWM_CHANNEL_A);
  ledcAttachPin(PWM_B_PIN, PWM_CHANNEL_B);
  pinMode(LIMIT_SWITCH_A, INPUT);
  pinMode(LIMIT_SWITCH_B, INPUT);
  stopMotor();

  // SPI通信の初期化
  pinMode(PIN_SPI_MOSI, OUTPUT);
  pinMode(PIN_SPI_MISO, INPUT);
  pinMode(PIN_SPI_SCK, OUTPUT);
  pinMode(PIN_SPI_SS_A, OUTPUT);
  pinMode(PIN_SPI_SS_B, OUTPUT);
  digitalWrite(PIN_SPI_SS_A, HIGH);
  digitalWrite(PIN_SPI_SS_B, HIGH);
  SPI.begin();
  SPI.setDataMode(SPI_MODE3); // SCKの立上りでデータを送受信、アイドル時はHIGH
  SPI.setBitOrder(MSBFIRST); // MSBから送信

  // L6470の初期設定
  L6470_setup(PIN_SPI_SS_A);
  L6470_setup(PIN_SPI_SS_B);
}

void loop() {
  // リミットスイッチ確認
  if (digitalRead(LIMIT_SWITCH_A) == HIGH && switch_A_enabled) {
    if (motor_running && direction) {
      stopMotor();
      switch_A_enabled = false;
      switch_B_enabled = true;
      Serial.println("Switch A activated. Waiting for Switch B.");
    }
  }

  if (digitalRead(LIMIT_SWITCH_B) == HIGH && switch_B_enabled) {
    if (motor_running && !direction) {
      stopMotor();
      switch_B_enabled = false;
      switch_A_enabled = true;
      Serial.println("Switch B activated. Waiting for Switch A.");
    }
  }

  // シリアル通信の処理
  if (Serial.available() > 0) {
    str = Serial.readString();
    str.trim();
    bool validCommand = false;

    // コマンドのフォーマットにコロンが含まれている場合
    if (str.indexOf(':') != -1) {
      int colonIndex = str.indexOf(':');
      String command = str.substring(0, colonIndex); // コマンド部分（例："A"）
      String angle_str = str.substring(colonIndex + 1); // 角度部分（例："30"）

      if (isNumber(angle_str)) {
        float angle = angle_str.toFloat();
        if (command == "A") {
          moveToAngle(PIN_SPI_SS_A, currentAngle_A, angle); // モータAを指定角度に回転
          validCommand = true;
        } else if (command == "B") {
          moveToAngle(PIN_SPI_SS_B, currentAngle_B, angle); // モータBを指定角度に回転
          validCommand = true;
        } else {
          Serial.println("Invalid motor command.");
        }
      } else {
        Serial.println("Invalid angle format.");
      }
    } else {
      // up, down コマンドの処理
      if (str == "up") {
        startForward();
        validCommand = true;
      } else if (str == "down") {
        startBackward();
        validCommand = true;
      } else if (str == "test") {
        // モータ駆動のテスト
        startForward();
        delay(1000); // 1秒間正転
        stopMotor();
        startBackward();
        delay(1000); // 1秒間逆転
        stopMotor();
        validCommand = true;
        Serial.println("test OK");
      } else {
        Serial.println("Unknown command.");
        Serial.println(str);
      }
    }

    if (validCommand) {
      Serial.println("READY");
    }
  }
}

// SPI通信で値を送信する関数
void L6470_send(unsigned char value, int ssPin) {
  digitalWrite(ssPin, LOW);
  SPI.transfer(value);
  digitalWrite(ssPin, HIGH);
}

// L6470の初期設定
void L6470_setup(int ssPin) {
  // 初期化コマンドをデバイスに送信
  L6470_send(0x00, ssPin);
  L6470_send(0x00, ssPin);
  L6470_send(0x00, ssPin);
  L6470_send(0xC0, ssPin);

  // 最大回転スピード設定（例：10bitの値）
  L6470_send(0x07, ssPin);
  L6470_send(0x20, ssPin);
}

// 指定角度まで回転させる関数
void moveToAngle(int ssPin, float &currentAngle, float targetAngle) {
  // ターゲット角度との差分を計算
  float angleDifference = targetAngle - currentAngle;

  // 必要なステップ数を計算
  int steps = abs(angleDifference) / STEP_ANGLE;

  // 回転方向を決定
  bool direction = angleDifference > 0;

  // 指定されたステップ数分モータを回転
  for (int i = 0; i < steps; i++) {
    L6470_send(direction ? 0x51 : 0x50, ssPin); // 正方向または逆方向の回転指令
    L6470_send(0x00, ssPin);                   // スピードデータ上位バイト
    L6470_send(0x20, ssPin);                   // スピードデータ中位バイト
    L6470_send(0x00, ssPin);                   // スピードデータ下位バイト
    delay(1);                                  // ステップ間の遅延
  }

  // 現在角度を更新
  currentAngle = targetAngle;

  // モータ停止
  L6470_send(0xB8, ssPin); // ハードストップ
  Serial.print("Motor moved to absolute ");
  Serial.print(targetAngle);
  Serial.println(" degrees.");
}

// モーターの正転
void startForward() {
  Serial.println("Motor moving forward");
  direction = true;
  motor_running = true;
  ledcWrite(PWM_CHANNEL_A, PWM_DUTY_CYCLE);
  ledcWrite(PWM_CHANNEL_B, 0);
}

// モーターの逆転
void startBackward() {
  Serial.println("Motor moving backward");
  direction = false;
  motor_running = true;
  ledcWrite(PWM_CHANNEL_A, 0);
  ledcWrite(PWM_CHANNEL_B, PWM_DUTY_CYCLE);
}

// モーターを停止
void stopMotor() {
  Serial.println("Motor stopped");
  motor_running = false;
  ledcWrite(PWM_CHANNEL_A, 0);
  ledcWrite(PWM_CHANNEL_B, 0);
}

// 数字かどうかを確認
bool isNumber(String str) {
  if (str.length() == 0) return false; // 空文字列は無効
  for (int i = 0; i < str.length(); i++) {
    if (!isdigit(str.charAt(i)) && str.charAt(i) != '.' && str.charAt(i) != '-') {
      return false;
    }
  }
  return true;
}
