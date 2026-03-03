from collections import deque
import cv2
import argparse
import pose
import display
import serial_stream
from utils import setup_logging, log


def parse_arguments():
    parser = argparse.ArgumentParser(description='Posture Monitor')
    parser.add_argument('--video', type=str, default=None,
                        help='Path to video file. Omit for webcam.')
    parser.add_argument('--neck-angle-limit', type=int, default=22,
                        help='Used for scoring (degrees).')
    parser.add_argument('--torso-angle-limit', type=int, default=18,
                        help='Used for scoring (degrees).')
    parser.add_argument('--avg-window', type=int, default=1,
                        help='Time window in seconds for average score.')
    parser.add_argument('--serial-port', type=str, default=None,
                        help='Serial port for ESP32 (e.g. /dev/tty.usbserial-0001). Omit to skip.')
    parser.add_argument('--baudrate', type=int, default=115200,
                        help='Serial baud rate.')
    parser.add_argument('--update-interval', type=float, default=0.5,
                        help='Interval in seconds for logging and serial updates.')
    parser.add_argument('--log-file', type=str, default='system_logs/posture_monitor.log',
                        help='Path to log file.')
    return parser.parse_args()


def main():
    args = parse_arguments()

    setup_logging(args.log_file)
    detector = pose.init()
    cap = cv2.VideoCapture(args.video if args.video else 0)

    # Open connection to esp32
    conn = serial_stream.connect(args.serial_port, args.baudrate) if args.serial_port else None

    fps = cap.get(cv2.CAP_PROP_FPS) or 30
    window_frames = int(fps * args.avg_window)
    score_history = deque(maxlen=window_frames)

    update_interval = int(fps * args.update_interval)
    frame_count = 0

    while True:
        success, frame = cap.read()
        if not success:
            print("Null.Frames")
            break

        result = pose.analyze(
            detector, frame,
            neck_limit=args.neck_angle_limit,
            torso_limit=args.torso_angle_limit,
        )

        if result is None:
            cv2.imshow('Posture Monitor', frame)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
            continue

        score_history.append(result["score"])
        result["avg_window"] = args.avg_window
        result["avg_score"] = int(sum(score_history) / len(score_history))

        display.draw(frame, result)

        # Log the result and send data to esp32
        frame_count += 1
        if frame_count % update_interval == 0:
            log(result)
            serial_stream.send(conn, result)

        cv2.imshow('Posture Monitor', frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # Close the connection to esp32
    serial_stream.disconnect(conn)
    cap.release()
    cv2.destroyAllWindows()


if __name__ == "__main__":
    main()