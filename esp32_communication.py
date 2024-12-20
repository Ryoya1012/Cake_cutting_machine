import cv2
import numpy as np
import math
import serial
import time

# グローバル変数
previous_angle = None
cumulative_rotation = 0
ALLOWED_ANGLE_ERROR = 10  # 許容誤差を10度と仮定

# ESP32とのシリアル通信を初期化
def initialize_esp32():
    try:
        esp32 = serial.Serial(port='COM3', baudrate=115200, timeout=1)
        print("ESP32に接続しました。")
        return esp32
    except serial.SerialException as e:
        print(f"ESP32に接続できません: {e}")
        exit()

# 分割線を描画する関数
def draw_dividing_lines(frame, center, radius, num_divisions, rotation_offset):
    angle_step = 360 / num_divisions
    for i in range(num_divisions):
        angle_deg = rotation_offset + i * angle_step
        angle_rad = math.radians(angle_deg)
        x_end = int(center[0] + radius * math.cos(angle_rad))
        y_end = int(center[1] + radius * math.sin(angle_rad))
        cv2.line(frame, center, (x_end, y_end), (0, 255, 0), 2)

# 回転角度を計算する関数
def calculate_rotation_angle(center, reference_point):
    global previous_angle, cumulative_rotation

    dx = reference_point[0] - center[0]
    dy = reference_point[1] - center[1]
    current_angle = math.degrees(math.atan2(dy, dx))

    if previous_angle is None:
        previous_angle = current_angle

    rotation_diff = current_angle - previous_angle

    # ±180°を超えた場合の調整
    if rotation_diff > 180:
        rotation_diff -= 360
    elif rotation_diff < -180:
        rotation_diff += 360

    cumulative_rotation += rotation_diff
    previous_angle = current_angle
    return cumulative_rotation

# ESP32にコマンドを送信する関数
def send_command_to_esp32(command, value=None):
    message = f"{command}:{value}\n" if value is not None else f"{command}\n"
    esp32.write(message.encode())
    print(f"ESP32に送信: {message.strip()}")

# ESP32からのコマンドを受信する関数
def receive_command_from_esp32():
    if esp32.in_waiting > 0:
        command = esp32.readline().decode().strip()
        print(f"ESP32から受信: {command}")
        return command
    return None

# ケーキの回転処理
def rotate_cake(center, radius, num_divisions):
    global cumulative_rotation

    for current_division in range(num_divisions):
        target_angle = 360 / num_divisions * current_division
        send_command_to_esp32("TARGET", target_angle)

        while True:
            # ESP32から回転完了の確認を受信
            command = receive_command_from_esp32()
            if command == "READY":
                # 現在の回転角度を計算
                reference_point = (center[0], center[1] - radius)
                current_angle = calculate_rotation_angle(center, reference_point)
                
                # 許容誤差内か確認
                angle_error = abs(target_angle - current_angle)
                if angle_error <= ALLOWED_ANGLE_ERROR:
                    print(f"目標角度 {target_angle}度に到達。誤差 {angle_error}度")
                    send_command_to_esp32("UP_to_DOWN")

                    # ESP32からStandbyを受信するまで待機
                    while True:
                        standby_command = receive_command_from_esp32()
                        if standby_command == "Standby":
                            print("次の目標角度に進みます。")
                            break
                    break
                else:
                    print(f"目標角度 {target_angle}度に未到達。誤差 {angle_error}度")
                    send_command_to_esp32("CORRECT", current_angle)
            else:
                # 他のESP32コマンドが来た場合はログを出力
                print(f"未処理のコマンド受信: {command}")
        
        send_command_to_esp32("COMPLETE", target_angle)
        print(f"目標角度 {target_angle}度を完了。次の分割へ。")

# メイン処理
def main():
    global esp32, cumulative_rotation

    # ESP32との接続を初期化
    esp32 = initialize_esp32()

    # 分割数を取得
    num_divisions = int(input("ケーキを何等分にしますか？: "))

    # カメラを起動
    cap = cv2.VideoCapture(1)  # ウェブカメラ
    if not cap.isOpened():
        print("ウェブカメラが見つかりません。")
        return

    while True:
        ret, frame = cap.read()
        if not ret:
            break

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

            reference_point = (x_center, y_center - radius)

            # 現在の回転角度を計算
            rotation_offset = calculate_rotation_angle(center, reference_point)

            # 円と分割線を描画
            cv2.circle(frame, center, radius, (255, 0, 0), 2)  # 円
            cv2.circle(frame, center, 5, (0, 0, 255), -1)  # 中心点
            draw_dividing_lines(frame, center, radius, num_divisions, rotation_offset)

            # ケーキの回転処理
            rotate_cake(center, radius, num_divisions)
            break

        cv2.imshow("ケーキ回転システム", frame)

        # 終了条件
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
