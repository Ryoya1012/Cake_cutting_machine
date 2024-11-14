#

import cv2
import numpy as np
import math

def draw_dividing_lines(frame, center, radius, num_divisions):
    # 円を等分する角度を計算
    angle_step = 360 / num_divisions

    for i in range(num_divisions):
        # 各分割点の角度（ラジアンに変換）
        angle_deg = i * angle_step
        angle_rad = math.radians(angle_deg)

        # 補助線の終点の座標を計算
        x_end = int(center[0] + radius * math.cos(angle_rad))
        y_end = int(center[1] + radius * math.sin(angle_rad))

        # 補助線を描画
        cv2.line(frame, center, (x_end, y_end), (0, 255, 0), 2)

def get_cake_center_and_shape(num_divisions):
    # ウェブカメラの映像を取得
    cap = cv2.VideoCapture(0)

    while True:
        # フレームを取得
        ret, frame = cap.read()
        if not ret:
            break

        # 画像をグレースケールに変換
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

        # 画像をぼかすことでノイズを低減
        blurred = cv2.GaussianBlur(gray, (5, 5), 0)

        # エッジを検出
        edges = cv2.Canny(blurred, 50, 150)

        # 輪郭を抽出
        contours, _ = cv2.findContours(edges, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

        # 輪郭が存在する場合のみ処理
        if contours:
            # 各輪郭をループ処理
            for contour in contours:
                # 輪郭の近似を行い、多角形の辺を取得
                epsilon = 0.02 * cv2.arcLength(contour, True)
                approx = cv2.approxPolyDP(contour, epsilon, True)

                # 円形の条件を確認 (辺の数が6以上なら円形に近いとみなす)
                if len(approx) > 12:
                    # ケーキの中心座標と半径を計算
                    (x, y), radius = cv2.minEnclosingCircle(contour)
                    center = (int(x), int(y))
                    radius = int(radius)

                    # 半径がある程度の大きさであれば描画
                    if radius > 10:
                        # ケーキの中心に円を描画
                        cv2.circle(frame, center, radius, (255, 0, 0), 2)

                        # 中心に赤いポインタを描画
                        cv2.circle(frame, center, 5, (0, 0, 255), -1)

                        # 中心座標を表示
                        cv2.putText(frame, f"Center: {center}", (center[0] - 50, center[1] - 10), 
                                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)

                        # 補助線を描画して円を分割
                        draw_dividing_lines(frame, center, radius, num_divisions)

        # フレームを表示
        cv2.imshow('Circle Division and Detection', frame)

        # 'q'キーが押されたら終了
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # ウェブカメラとウィンドウを解放
    cap.release()
    cv2.destroyAllWindows()

# プログラムを実行（例として8等分）
get_cake_center_and_shape(num_divisions=8)
