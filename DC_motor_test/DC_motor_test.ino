// ピン定義
#define PWM_A_PIN 14           // モータードライバのPWM Aピン
#define PWM_B_PIN 27           // モータードライバのPWM Bピン

// PWM設定
#define PWM_CHANNEL_A 0        // PWMチャンネルA
#define PWM_CHANNEL_B 1        // PWMチャンネルB
#define PWM_FREQUENCY 5000     // 5kHz
#define PWM_RESOLUTION 8       // 8ビット
#define PWM_DUTY_CYCLE 100     // PWMのデューティサイクル (0-255)

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

  // モーターを停止状態で初期化
  stopMotor();
}

void loop() {
  // シリアル通信からの入力処理
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim(); // 空白文字を削除

    if (command == "up") {
      startForward();
    } else if (command == "down") {
      startBackward();
    } else {
      stopMotor();
    }
  }
}

// モーターを正転させる
void startForward() {
  Serial.println("Motor moving forward");
  ledcWrite(PWM_CHANNEL_A, PWM_DUTY_CYCLE); // PWM Aに信号を出力
  ledcWrite(PWM_CHANNEL_B, 0);             // PWM Bを停止
}

// モーターを逆転させる
void startBackward() {
  Serial.println("Motor moving backward");
  ledcWrite(PWM_CHANNEL_A, 0);             // PWM Aを停止
  ledcWrite(PWM_CHANNEL_B, PWM_DUTY_CYCLE); // PWM Bに信号を出力
}

// モーターを停止する
void stopMotor() {
  Serial.println("Motor stopped");
  ledcWrite(PWM_CHANNEL_A, 0);
  ledcWrite(PWM_CHANNEL_B, 0);
}
