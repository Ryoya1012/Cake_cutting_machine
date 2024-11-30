// ピン定義（必要に応じて変更可能）
#define PWM_PIN 16           // PWM信号ピン
#define DIR_PIN 17           // 回転方向ピン
#define LIMIT_SWITCH_A 18    // リミットスイッチA
#define LIMIT_SWITCH_B 19    // リミットスイッチB

// PWM設定
#define PWM_CHANNEL 0
#define PWM_FREQUENCY 5000   // 5kHz
#define PWM_RESOLUTION 8     // 8ビット
#define PWM_DUTY_CYCLE 200   // デフォルトのPWMデューティサイクル (0-255)

// 状態変数
bool switchA_pressed = false;
bool switchB_pressed = false;
bool moving_forward = false;
bool moving_backward = false;

void setup() {
  // ピンモード設定
  pinMode(PWM_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(LIMIT_SWITCH_A, INPUT_PULLUP);
  pinMode(LIMIT_SWITCH_B, INPUT_PULLUP);

  // PWM初期化
  ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcAttachPin(PWM_PIN, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, 0); // モーター停止

  // シリアル通信初期化
  Serial.begin(115200);
  Serial.println("System Initialized");
}

void loop() {
  // シリアル通信からの入力処理
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    if (command == "A") {
      startForward();
    } else if (command == "B") {
      startBackward();
    }
  }

  // リミットスイッチAの処理
  if (digitalRead(LIMIT_SWITCH_A) == LOW && !switchA_pressed) {
    switchA_pressed = true;
    stopMotor();
    Serial.println("Limit Switch A activated");
  } else if (digitalRead(LIMIT_SWITCH_A) == HIGH && switchA_pressed) {
    switchA_pressed = false;
  }

  // リミットスイッチBの処理
  if (digitalRead(LIMIT_SWITCH_B) == LOW && !switchB_pressed) {
    switchB_pressed = true;
    stopMotor();
    Serial.println("Limit Switch B activated");
  } else if (digitalRead(LIMIT_SWITCH_B) == HIGH && switchB_pressed) {
    switchB_pressed = false;
  }
}

// モーターを前進させる
void startForward() {
  if (!switchA_pressed) { // リミットスイッチAが押されていない場合のみ動作
    digitalWrite(DIR_PIN, HIGH);
    ledcWrite(PWM_CHANNEL, PWM_DUTY_CYCLE);
    moving_forward = true;
    moving_backward = false;
    Serial.println("Motor moving forward");
  }
}

// モーターを後退させる
void startBackward() {
  if (!switchB_pressed) { // リミットスイッチBが押されていない場合のみ動作
    digitalWrite(DIR_PIN, LOW);
    ledcWrite(PWM_CHANNEL, PWM_DUTY_CYCLE);
    moving_backward = true;
    moving_forward = false;
    Serial.println("Motor moving backward");
  }
}

// モーターを停止させる
void stopMotor() {
  ledcWrite(PWM_CHANNEL, 0); // PWM信号を0にして停止
  moving_forward = false;
  moving_backward = false;
  Serial.println("Motor stopped");
}
