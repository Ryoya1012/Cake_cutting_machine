import cv2
import numpy as np

def get_cake_center_and_shape():
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
            # 最も大きな輪郭を取得
            largest_contour = max(contours, key=cv2.contourArea)

            # 最大輪郭の面積が十分大きい場合に処理を行う
            if cv2.contourArea(largest_contour) > 500:
                # 輪郭の近似を行い、多角形の辺を取得
                epsilon = 0.02 * cv2.arcLength(largest_contour, True)
                approx = cv2.approxPolyDP(largest_contour, epsilon, True)

                # 円形に近いかどうかを確認する条件
                if len(approx) > 7:
                    # ケーキの中心座標と半径を計算
                    (x, y), radius = cv2.minEnclosingCircle(largest_contour)
                    center = (int(x), int(y))
                    radius = int(radius)

                    # ケーキの中心に円を描画
                    cv2.circle(frame, center, radius, (255, 0, 0), 2)
                    cv2.circle(frame, center, 5, (0, 0, 255), -1)

                    # 中心座標を表示
                    cv2.putText(frame, f"Center: {center}", (center[0] - 50, center[1] - 10), 
                                cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)

                # 最大の輪郭を描画
                cv2.drawContours(frame, [largest_contour], -1, (0, 255, 0), 2)

        # フレームを表示
        cv2.imshow('Cake Shape and Contour Detection', frame)

        # 'q'キーが押されたら終了
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # ウェブカメラとウィンドウを解放
    cap.release()
    cv2.destroyAllWindows()

# プログラムを実行
get_cake_center_and_shape()
