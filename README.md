# Cake_cutting_machine

## The purpose
 The robot recognizes the cake and performs actions such as cutting and rotating the cake.

## Specification
 - Vision-related
    - Cake_cutting_opencv : Python opencv

 - ESP32-related
    - Moving_motorv : Arduino IDE(using MPU : ESP32)


## Program
 -esp32_communication.py
   このコードは, ウェブカメラでケーキの回転システムを制御するプログラムになっています.
   ケーキを指定した分割数に基づき等分線を計算し, 各等分線にナイフの刃先が来るようにESP32と知るある通信を用いてその回転を制御する.

### 処理の流れ
   - 1. カメラ映像のキャプチャ
      ・OpenCVを使用してカメラ映像をキャプチャし, 画像を処理する
      ・cv2.HoughCirclesを使用して円を検出する. 検出した円の中心と半径を用いて回転を計算する.
   
   - 2. 回転角度の計算
      ・円の中心と参照点(円周上の1点)を使用して, 回転角度を計算する.
      ・現在の回転角度と前回の角度を基に, 累積回転角度を計算する.
   
   - 3. ESP32との通信
      ・ケーキを回転(サーボモータを使用し)させるために, 目標角度をESP32に送信.
      ・ESP32は回転が完了すると"READY"のメッセージを返し, 次の目標角度まで回転する.
   
   - 4. 分割線の描画
      ・円と分割線を描画して, 現在の回転状態を可視化する.
   
   - 5. 回転完了後
      ・すべての目標角度に到達した後, 回転処理が完了する.