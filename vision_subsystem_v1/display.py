import cv2

# Colors (BGR).
GREEN = (127, 255, 0)
RED = (50, 50, 255)
WHITE = (255, 255, 255)
FONT = cv2.FONT_HERSHEY_SIMPLEX

SKELETON = [
    ("l_shoulder", "l_ear"),
    ("l_hip", "l_shoulder"),
    ("r_shoulder", "r_ear"),
    ("r_hip", "r_shoulder"),
]


def draw(frame, result):
    """
    Draw posture overlay on the frame.

    Args:
        frame: BGR image
        result: Result dict from pose.analyze().
    """
    h, w = frame.shape[:2]
    landmarks = result["landmarks"]

    # Draw skeleton lines (always red).
    for base, tip in SKELETON:
        if base in landmarks and tip in landmarks:
            cv2.line(frame, landmarks[base][:2], landmarks[tip][:2], RED, 2)

    # Draw white circle dots on all landmarks.
    for name, pt in landmarks.items():
        cv2.circle(frame, pt[:2], 7, WHITE, 2)

    # Text at top-right corner .
    x = w - 280
    cv2.putText(frame, f"Score: {result['score']}/100", (x, 30), FONT, 0.7, RED, 2)
    cv2.putText(frame, f"{result['avg_window']}s Avg Score: {result['avg_score']}/100", (x, 60), FONT, 0.7, RED, 2)
    cv2.putText(frame, f"Neck Inclined Angle: {int(result['neck_angle'])} deg", (x, 90), FONT, 0.6, RED, 2)
    cv2.putText(frame, f"Torso Inclined Angle: {int(result['torso_angle'])} deg", (x, 120), FONT, 0.6, RED, 2)
