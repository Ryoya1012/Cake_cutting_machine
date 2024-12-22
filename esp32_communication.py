import cv2
import numpy as np
import serial
import time
import math
import pygame  # pygameライブラリをインポート
import threading  # スレッド用モジュールをインポート

# 設定
ESP32_PORT = 'COM7'  # ESP32のシリアルポート
BAUD_RATE = 115200   # ボーレート
TIMEOUT = 16         # タイムアウト時間（秒）
SOUND_FILE_PATH = "C:\\Users\\sator\\Downloads\\Warning-Siren03-mp3\\Warning-Siren03-02(High-Long).mp3"  # 音声ファイルのパス

# ESP32との通信を初期化
def initialize_esp32():
    try:
        esp32 = serial.Serial(port=ESP32_PORT, baudrate=BAUD_RATE, timeout=TIMEOUT)
        print("ESP32に接続しました。")
        return esp32
    except serial.SerialException as e:
        print(f"ESP32に接続できません: {e}")
        exit()

# ESP32にコマンドを送信
def send_command_to_esp32(esp32, command, value=None):
    message = f"{command}:{value}\n" if value is not None else f"{command}\n"
    esp32.write(message.encode())
    print(f"ESP32に送信: {message.strip()}")

# ESP32からの応答を受信
def receive_response_from_esp32(esp32):
    response = None
    decoded_response = ""  # 初期化
    while esp32.in_waiting > 0:
        response = esp32.readline()
        
        # バイナリデータをチェックして無視する
        try:
            decoded_response = response.decode().strip()
        except UnicodeDecodeError:
            print("Non-UTF-8 data received, ignoring this message.")
            decoded_response = ""  # 無視して空文字列を返す
        
        if decoded_response:
            print(f"ESP32から受信: {decoded_response}")
    
    return decoded_response

# 分割線を描画する関数
def draw_dividing_lines(frame, center, radius, num_divisions):
    angle_step = 360 / num_divisions
    for i in range(num_divisions):
        angle_deg = i * angle_step
        angle_rad = math.radians(angle_deg)
        x_end = int(center[0] + radius * math.cos(angle_rad))
        y_end = int(center[1] + radius * math.sin(angle_rad))
        cv2.line(frame, center, (x_end, y_end), (0, 255, 0), 2)

# 音声ファイルを再生
def play_sound(file_path):
    pygame.mixer.init()  # pygameのミキサーを初期化
    pygame.mixer.music.load(file_path)  # 音声ファイルをロード
    pygame.mixer.music.play()  # 音を再生
    while pygame.mixer.music.get_busy():  # 音声が再生されている間は待機
        time.sleep(0.1)

# 回転準備完了を受信
def wait_for_ready(esp32):
    response = None
    while True:
        response = receive_response_from_esp32(esp32)
        if response == "READY":
            print("回転準備完了")
            play_sound(SOUND_FILE_PATH)  # 音を鳴らす
            break
        elif response == "ERROR":
            print("エラーが発生しました。")
            return False
    return True

# ケーキの回転処理
def rotate_cake(esp32, center, radius, num_divisions, cap):
    # 最初のターゲット角度0を送信
    target_angle = 0
    send_command_to_esp32(esp32, "TARGET", target_angle)
    print(f"\n目標角度: {target_angle}度")  # 目標角度 0 度を送信

    # ESP32から準備完了を受信
    if not wait_for_ready(esp32):
        return

    # 分割ごとに回転処理を実行
    for current_division in range(1, num_divisions):  # 最初のターゲット0は送信済み
        target_angle = round(360 / num_divisions * current_division, 1)  # 角度を小数点1桁に丸める
        print(f"\n目標角度: {target_angle:.1f}度")  # 小数点1桁で表示

        # 目標角度をESP32に送信
        send_command_to_esp32(esp32, "TARGET", target_angle)

        # ESP32から準備完了を受信
        if not wait_for_ready(esp32):
            return

        # 次の分割へ進む準備
        print("次の分割に進みます。")
        play_sound("C:\\Users\\sator\\Downloads\\Warning-Siren03-mp3\\Warning-Siren03-02(High-Long).mp3")  # 次の分割への進行を音で通知

# カメラからケーキを認識する処理
def detect_cake_and_process(esp32, num_divisions):
    cap = cv2.VideoCapture(1)  # 使用するカメラのインデックスを変更してみる（0, 1, 2など）
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)  # 幅を640に設定
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)  # 高さを480に設定

    if not cap.isOpened():
        print("ウェブカメラが見つかりません。")
        return

    print("カメラが正常にオープンされました。")

    # 音声再生をスレッドで並行して実行
    sound_thread = threading.Thread(target=play_sound, args=(SOUND_FILE_PATH,))
    sound_thread.start()

    while True:
        ret, frame = cap.read()
        if not ret:
            print("カメラ映像の取得に失敗しました。")
            break
        else:
            print("フレームを取得しました。")

        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        gray = cv2.medianBlur(gray, 5)

        # 円検出
        circles = cv2.HoughCircles(
            gray,
            cv2.HOUGH_GRADIENT,
            dp=1.2,
            minDist=50,
            param1=100,
            param2=30,
            minRadius=50,
            maxRadius=300
        )

        if circles is not None:
            circles = np.round(circles[0, :]).astype("int")
            x_center, y_center, radius = circles[0]
            center = (x_center, y_center)

            # 分割線を描画
            draw_dividing_lines(frame, center, radius, num_divisions)
            cv2.imshow("Cake", frame)

            # 回転処理の実行
            rotate_cake(esp32, center, radius, num_divisions, cap)
            break

        cv2.imshow("Cake", frame)

        # 終了条件
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()

# メイン処理
def main():
    # ESP32との接続を初期化
    esp32 = initialize_esp32()

    # 開始待機
    print("処理を始めるには 'A' を入力してください。")
    while True:
        start_command = input("コマンド入力: ").strip().upper()
        if start_command == 'A':
            print("処理を開始します。")
            break
        else:
            print("無効な入力です。'A' を入力してください。")

    # 分割数を取得
    try:
        num_divisions = int(input("ケーキを何等分にしますか？: "))
    except ValueError:
        print("正しい数値を入力してください。")
        return

    # ケーキ認識と回転処理
    detect_cake_and_process(esp32, num_divisions)

    # シリアル通信を閉じる
    esp32.close()
    print("システムを終了します。")

if __name__ == "__main__":
    main()
