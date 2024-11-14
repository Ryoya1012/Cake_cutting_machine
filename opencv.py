import cv2
import numpy as np
import math

# Tracking function to detect cake shape and calculate rotation angle
def tracking_line(frame, track_window, term_crit):
    # 映像フレームを HSV カラースペースに変換（赤色の検出に有効）
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    
    # 赤色の範囲を定義（赤色のオブジェクトのみを検出するため）
    lower_red = np.array([0, 120, 70])
    upper_red = np.array([10, 255, 255])
    
    # 赤色の部分をマスクで抽出
    mask = cv2.inRange(hsv, lower_red, upper_red)
    
    # CamShift を使用して追跡窓（track_window）内で赤色領域を追跡し、回転矩形の位置を取得
    ret, track_window = cv2.CamShift(mask, track_window, term_crit)
    
    # 追跡した矩形の頂点座標を取得し、描画用に整数に変換
    pts = cv2.boxPoints(ret)
    pts = np.int64(pts)
    
    # 追跡矩形の輪郭を黄色で描画
    cv2.polylines(frame, [pts], True, (0, 255, 255), 2)

    # 回転角度を計算（矩形の1辺の傾きから計算）
    dx = pts[1][0] - pts[0][0]
    dy = pts[1][1] - pts[0][1]
    angle_rad = math.atan2(dy, dx)  # 傾きをラジアンで計算
    angle_deg = math.degrees(angle_rad)  # 度に変換

    # 矩形の中心座標を計算
    x_center = int((pts[0][0] + pts[2][0]) / 2)
    y_center = int((pts[0][1] + pts[2][1]) / 2)
    
    # 中心座標、回転角度、更新された track_window を返す
    return (x_center, y_center), angle_deg, track_window

# Draw dividing lines that rotate with the cake
def draw_dividing_lines(frame, center, radius, num_divisions, start_angle_deg):
    # 各分割線の角度を計算（分割数に基づく）
    angle_step = 360 / num_divisions
    for i in range(num_divisions):
        # 各分割線の角度を計算（開始角度に加算）
        angle_deg = start_angle_deg + i * angle_step
        angle_rad = math.radians(angle_deg)
        
        # 分割線の終点を計算（円の中心から radius の距離）
        x_end = int(center[0] + radius * math.cos(angle_rad))
        y_end = int(center[1] + radius * math.sin(angle_rad))
        
        # 緑色の分割線を描画
        cv2.line(frame, center, (x_end, y_end), (0, 255, 0), 2)

# Main function
def get_cake_center_and_shape(num_divisions):
    # カメラ映像のキャプチャオブジェクトを作成
    cap = cv2.VideoCapture(0)
    
    # 初期フレームの取得と画面サイズの取得
    ret, frame = cap.read()
    h, w, _ = frame.shape
    
    # 追跡窓の初期設定（画面中央に 100x100 の矩形を設定）
    track_window = (w//2 - 50, h//2 - 50, 100, 100)
    
    # CamShift の終了条件（10回または変化量が1未満で終了）
    term_crit = (cv2.TERM_CRITERIA_EPS | cv2.TERM_CRITERIA_COUNT, 10, 1)
    
    # 分割線を描く円の半径を設定
    radius = 150

    while True:
        # 新しいフレームの読み込み
        ret, frame = cap.read()
        if not ret:
            break
        
        # 追跡されたケーキの中心座標と回転角度を取得
        tracked_center, start_angle_deg, track_window = tracking_line(frame, track_window, term_crit)
        
        # 円の描画（追跡されたケーキの中心を基準）
        cv2.circle(frame, tracked_center, radius, (255, 0, 0), 2)
        
        # ケーキの中心に小さな赤い点を描画
        cv2.circle(frame, tracked_center, 5, (0, 0, 255), -1)
        
        # 回転角度に基づいた分割線の描画
        draw_dividing_lines(frame, tracked_center, radius, num_divisions, start_angle_deg)
        
        # 結果を表示
        cv2.imshow('Tracked Cake Shape and Dividing Lines', frame)
        
        # "q" キーが押されたらループ終了
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # カメラを解放し、ウィンドウを閉じる
    cap.release()
    cv2.destroyAllWindows()

# ターミナルから分割数を入力
while True:
    try:
        num_divisions = int(input("ケーキを何等分にしますか？整数を入力してください: "))
        if num_divisions > 0:
            break
        else:
            print("正の整数を入力してください。")
    except ValueError:
        print("無効な入力です。整数を入力してください。")

# Run the program
get_cake_center_and_shape(num_divisions)
