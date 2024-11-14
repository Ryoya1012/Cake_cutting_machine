import cv2
import numpy as np
import math
import mss

# Variables to keep track of previous frame's angle
previous_angle = None
cumulative_rotation = 0

# Draw dividing lines that rotate with the detected circle
def draw_dividing_lines(frame, center, radius, num_divisions, rotation_offset):
    angle_step = 360 / num_divisions
    for i in range(num_divisions):
        # Calculate each division line angle with offset
        angle_deg = rotation_offset + i * angle_step
        angle_rad = math.radians(angle_deg)
        
        # Calculate the endpoint of the dividing line
        x_end = int(center[0] + radius * math.cos(angle_rad))
        y_end = int(center[1] + radius * math.sin(angle_rad))
        
        # Draw the dividing line in green
        cv2.line(frame, center, (x_end, y_end), (0, 255, 0), 2)

# Calculate the rotation based on the detected circle and feature
def calculate_rotation_angle(center, reference_point):
    global previous_angle, cumulative_rotation
    
    # Calculate angle from the circle center to the reference point
    dx = reference_point[0] - center[0]
    dy = reference_point[1] - center[1]
    current_angle = math.degrees(math.atan2(dy, dx))

    # Initialize previous_angle on the first call
    if previous_angle is None:
        previous_angle = current_angle

    # Calculate rotation difference between frames
    rotation_diff = current_angle - previous_angle

    # Adjust for wrap-around (e.g., from 359 degrees back to 0)
    if rotation_diff > 180:
        rotation_diff -= 360
    elif rotation_diff < -180:
        rotation_diff += 360

    # Update cumulative rotation and previous angle
    cumulative_rotation += rotation_diff
    previous_angle = current_angle

    # Print the current cumulative rotation
    print(f"ケーキの累積回転角度: {cumulative_rotation:.2f}度")

    return cumulative_rotation

# Main function to capture the screen, detect circles, and track rotation in real-time
def get_screen_center_and_shape(num_divisions):
    with mss.mss() as sct:
        monitor = sct.monitors[1]  # Capture the main monitor

        while True:
            screenshot = sct.grab(monitor)
            frame = np.array(screenshot)
            
            # Convert color space to grayscale for circle detection
            gray = cv2.cvtColor(frame, cv2.COLOR_BGRA2GRAY)
            gray = cv2.medianBlur(gray, 5)
            
            # Detect circles in the image using HoughCircles
            circles = cv2.HoughCircles(
                gray, 
                cv2.HOUGH_GRADIENT, 
                dp=1.2, 
                minDist=50, 
                param1=100, 
                param2=30, 
                minRadius=10, 
                maxRadius=300
            )

            if circles is not None:
                circles = np.round(circles[0, :]).astype("int")
                
                # Assume the first detected circle is the one we want to track
                x_center, y_center, radius = circles[0]
                tracked_center = (x_center, y_center)

                # Choose a reference point (e.g., top of the circle) to track rotation
                reference_point = (x_center, y_center - radius)

                # Calculate the current rotation angle
                rotation_offset = calculate_rotation_angle(tracked_center, reference_point)

                # Draw the detected circle and dividing lines that rotate with it
                cv2.circle(frame, tracked_center, radius, (255, 0, 0), 2)  # Draw the circle
                cv2.circle(frame, tracked_center, 5, (0, 0, 255), -1)  # Draw the center point
                draw_dividing_lines(frame, tracked_center, radius, num_divisions, rotation_offset)

            # Display the frame
            cv2.imshow('Real-Time Circle Detection with Rotating Dividing Lines', frame)
            
            # Exit on "q" key press
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

    cv2.destroyAllWindows()

# Get the number of divisions from the user
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
get_screen_center_and_shape(num_divisions)
