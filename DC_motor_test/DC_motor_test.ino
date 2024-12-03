// ピン定義
#define PWM_A_PIN 27           // モータードライバのPWM Aピン
#define PWM_B_PIN 14           // モータードライバのPWM Bピン
#define LIMIT_SWITCH_A 26      // リミットスイッチAのピン
#define LIMIT_SWITCH_B 32      // リミットスイッチBのピン

// PWM設定
#define PWM_CHANNEL_A 0        // PWMチャンネルA
#define PWM_CHANNEL_B 1        // PWMチャンネルB
#define PWM_FREQUENCY 5000     // 5kHz
#define PWM_RESOLUTION 8       // 8ビット
#define PWM_DUTY_CYCLE 200     // PWMのデューティサイクル (0-255)

// グローバル変数
bool direction = true;        // true: 正転, false: 逆転
bool motor_running = true;    // モーターが動作中かどうか
bool switchA_pressed = false; // スイッチAが押されているか
bool switchB_pressed = false; // スイッチBが押されているか

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

  // スイッチのピンを設定
  pinMode(LIMIT_SWITCH_A, INPUT_PULLUP);
  pinMode(LIMIT_SWITCH_B, INPUT_PULLUP);

  // モーターを停止状態で初期化
  stopMotor();
}

void loop() {
  // 正転の場合はスイッチAのみ確認
  if (direction) {
    if (digitalRead(LIMIT_SWITCH_A) == LOW) { // スイッチAが押された場合
      if (!switchA_pressed) { // 押された瞬間のみ処理
        stopMotor();
        Serial.println("Switch A activated. Motor stopped.");
        switchA_pressed = true;
      }
    } else {
      switchA_pressed = false; // スイッチAが押されていない状態を記録
    }
  }

  // 逆転の場合はスイッチBのみ確認
  if (!direction) {
    if (digitalRead(LIMIT_SWITCH_B) == LOW) { // スイッチBが押された場合
      if (!switchB_pressed) { // 押された瞬間のみ処理
        stopMotor();
        Serial.println("Switch B activated. Motor stopped.");
        switchB_pressed = true;
      }
    } else {
      switchB_pressed = false; // スイッチBが押されていない状態を記録
    }
  }

  // シリアル通信からの入力処理
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    if (command.length() > 0) {
      reverseMotor(); // 入力があればモーターの方向を反転して再開
    }
  }

  // モーターが動作中の場合のみPWMを出力
  if (motor_running) {
    if (direction) {
      // 正転
      ledcWrite(PWM_CHANNEL_A, PWM_DUTY_CYCLE);
      ledcWrite(PWM_CHANNEL_B, 0);
    } else {
      // 逆転
      ledcWrite(PWM_CHANNEL_A, 0);
      ledcWrite(PWM_CHANNEL_B, PWM_DUTY_CYCLE);
    }
  }
}

// モーターを停止する
void stopMotor() {
  ledcWrite(PWM_CHANNEL_A, 0);
  ledcWrite(PWM_CHANNEL_B, 0);
  motor_running = false;
}

// モーターの方向を反転して再開する
void reverseMotor() {
  direction = !direction; // 方向を反転
  motor_running = true;   // モーターを再開
  if (direction) {
    Serial.println("Motor restarted: Forward");
  } else {
    Serial.println("Motor restarted: Backward");
  }
}
