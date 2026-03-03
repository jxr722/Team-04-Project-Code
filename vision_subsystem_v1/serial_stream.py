import time
import serial


def connect(port, baudrate=115200):
    """
    Open serial connection to ESP32.
    Waits 2s after opening to allow ESP32 to reset.

    Returns:
        serial.Serial connection object, or None if failed.
    """
    try:
        conn = serial.Serial(port, baudrate, timeout=1)
        time.sleep(2)
        print(f"Serial connected: {port} @ {baudrate}")
        return conn
    except serial.SerialException as e:
        print(f"Serial connection failed: {e}")
        return None


def send(conn, result, min_interval=0.5, _state={"last_send": 0}):
    """
    Send posture data to ESP32 as CSV: "score,avg_score,neck_angle,torso_angle\n"
    Rate-limited to one send per min_interval seconds.

    Args:
        conn: serial.Serial object from connect(), or None.
        result: Result dict from pose.analyze() (with avg_score added).
        min_interval: Minimum seconds between sends.
    """
    if conn is None:
        return

    now = time.time()
    if now - _state["last_send"] < min_interval:
        return

    line = "{},{},{},{}\n".format(
        result["score"],
        result["avg_score"],
        int(result["neck_angle"]),
        int(result["torso_angle"]),
    )

    try:
        conn.write(line.encode())
        _state["last_send"] = now
    except serial.SerialException:
        print("Serial send failed (device disconnected?)")


def disconnect(conn):
    """Close the serial connection."""
    if conn is not None:
        conn.close()
        print("Serial disconnected.")