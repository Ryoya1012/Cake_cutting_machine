import cv2
import numpy as np
import math

# Tracking function
def tracking_line(frame, track_window, term_crit):
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    lower_red = np.array([0, 120, 70])
    upper_red = np.array([10, 255, 255])
    mask = cv2.inRange(hsv, lower_red, upper_red)
    ret, track_window = cv2.CamShift(mask, track_window, term_crit)
    pts = cv2.boxPoints(ret)
    pts = np.int64(pts)
    cv2.polylines(frame, [pts], True, (0, 255, 255), 2)
    x_center = int((pts[0][0] + pts[2][0]) / 2)
    y_center = int((pts[0][1] + pts[2][1]) / 2)
    return (x_center, y_center), track_window

# Drawing dividing lines
def draw_dividing_lines(frame, center, radius, num_divisions):
    angle_step = 360 / num_divisions
    for i in range(num_divisions):
        angle_deg = i * angle_step
        angle_rad = math.radians(angle_deg)
        x_end = int(center[0] + radius * math.cos(angle_rad))
        y_end = int(center[1] + radius * math.sin(angle_rad))
        cv2.line(frame, center, (x_end, y_end), (0, 255, 0), 2)

# Main function
def get_cake_center_and_shape(num_divisions):
    cap = cv2.VideoCapture(0)
    ret, frame = cap.read()
    h, w, _ = frame.shape
    track_window = (w//2 - 50, h//2 - 50, 100, 100)
    term_crit = (cv2.TERM_CRITERIA_EPS | cv2.TERM_CRITERIA_COUNT, 10, 1)
    center = (w // 2, h // 2)
    radius = 150

    while True:
        ret, frame = cap.read()
        if not ret:
            break
        tracked_center, track_window = tracking_line(frame, track_window, term_crit)
        cv2.circle(frame, center, radius, (255, 0, 0), 2)
        cv2.circle(frame, tracked_center, 5, (0, 0, 255), -1)
        draw_dividing_lines(frame, center, radius, num_divisions)
        cv2.imshow('Tracked Circle and Dividing Lines', frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()

# Run the program
num_divisions = 6
get_cake_center_and_shape(num_divisions)
