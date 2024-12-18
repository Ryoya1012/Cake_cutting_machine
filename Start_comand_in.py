def main():
    global esp32, cumulative_rotation

    # ESP32との接続を初期化
    esp32 = initialize_esp32()

    # 分割数を取得
    num_divisions = int(input("ケーキを何等分にしますか？: "))

    # 'A'を受信するまで待機
    print("ESP32から'A'が送信されるのを待機中...")
    while True:
        command = receive_command_from_esp32()
        if command == 'A':
            print("ESP32から'A'を受信しました。処理を開始します。")
            break

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

    # 全ての処理が完了したことを表示
    print("ALL TASK IS FINISH")

if __name__ == "__main__":
    main()
