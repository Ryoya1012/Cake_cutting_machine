// ピン定義
#define PWM_A_PIN 14           // モータードライバのPWM Aピン
#define PWM_B_PIN 27           // モータードライバのPWM Bピン
#define LIMIT_SWITCH_A 32      // リミットスイッチAのピン
#define LIMIT_SWITCH_B 26      // リミットスイッチBのピン

// PWM設定
#define PWM_CHANNEL_A 0        // PWMチャンネルA
#define PWM_CHANNEL_B 1        // PWMチャンネルB
#define PWM_FREQUENCY 5000     // 5kHz
#define PWM_RESOLUTION 8       // 8ビット
#define PWM_DUTY_CYCLE 100     // PWMのデューティサイクル (0-255)

// グローバル変数
bool motor_running = false;    // モーターが動作中かどうか
bool direction = true;         // true: 正転, false: 逆転
bool switch_A_enabled = true;  // スイッチAが有効かどうか
bool switch_B_enabled = true;  // スイッチBが有効かどうか

void setup() {
  // シリアル通信初期化
  Serial.begin(115200);
  Serial.println("Motor Control Initialized");

  // PWMの初期化
  ledcSetup(PWM_CHANNEL_A, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_B, PWM_FREQUENCY, PWM_RESOLUTION);

  // PWMチャンネルをピンにアタッチ
  ledcAttachPin(PWM_A_PIN, PWM_CHANNEL_A);
  ledcAttachPin(PWM_B_PIN, PWM_CHANNEL_B);

  // スイッチのピンを設定（プルダウン回路を前提）
  pinMode(LIMIT_SWITCH_A, INPUT);
  pinMode(LIMIT_SWITCH_B, INPUT);

  // モーターを停止状態で初期化
  stopMotor();
}

void loop() {
  // スイッチAの確認
  if (digitalRead(LIMIT_SWITCH_A) == HIGH && switch_A_enabled) {
    if (motor_running && direction) { // 正転中にスイッチAが反応
      stopMotor();
      switch_A_enabled = false; // スイッチAを無効化
      switch_B_enabled = true;  // スイッチBを有効化
      Serial.println("Switch A activated. Waiting for Switch B.");
    }
  }

  // スイッチBの確認
  if (digitalRead(LIMIT_SWITCH_B) == HIGH && switch_B_enabled) {
    if (motor_running && !direction) { // 逆転中にスイッチBが反応
      stopMotor();
      switch_B_enabled = false; // スイッチBを無効化
      switch_A_enabled = true;  // スイッチAを有効化
      Serial.println("Switch B activated. Waiting for Switch A.");
    }
  }

  // シリアル通信からの入力処理
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim(); // 空白文字を削除

    if (switch_B_enabled) {
      // スイッチB有効時に "down" コマンドを受け付ける
      if (command == "down") {
        startBackward();
      } else {
        Serial.println("Invalid command. Waiting for Switch B.");
      }
    } else if (switch_A_enabled) {
      // スイッチA有効時に "up" コマンドを受け付ける
      if (command == "up") {
        startForward();
      } else {
        Serial.println("Invalid command. Waiting for Switch A.");
      }
    } else {
      // 通常動作
      if (command == "up") {
        startForward();
      } else if (command == "down") {
        startBackward();
      } else {
        stopMotor();
      }
    }
  }
}

// モーターを正転させる
void startForward() {
  Serial.println("Motor moving forward");
  direction = true;
  motor_running = true;
  ledcWrite(PWM_CHANNEL_A, PWM_DUTY_CYCLE); // PWM Aに信号を出力
  ledcWrite(PWM_CHANNEL_B, 0);             // PWM Bを停止
}

// モーターを逆転させる
void startBackward() {
  Serial.println("Motor moving backward");
  direction = false;
  motor_running = true;
  ledcWrite(PWM_CHANNEL_A, 0);             // PWM Aを停止
  ledcWrite(PWM_CHANNEL_B, PWM_DUTY_CYCLE); // PWM Bに信号を出力
}

// モーターを停止する
void stopMotor() {
  Serial.println("Motor stopped");
  motor_running = false;
  ledcWrite(PWM_CHANNEL_A, 0);
  ledcWrite(PWM_CHANNEL_B, 0);
}
