import serial
import time

# シリアルポートとボーレートを設定
ser = serial.Serial('COM3', 115200, timeout=1)  # COMポート番号を適宜変更
time.sleep(2)  # ESP32の初期化待ち

# データの送信
data_to_send = "Hello, ESP32!"
ser.write((data_to_send + '\n').encode())  # データ送信
print("Sent:", data_to_send)

# 応答を待機して受信
response = ser.readline().decode().strip()
print("Received from ESP32:", response)

# シリアル通信を閉じる
ser.close()
