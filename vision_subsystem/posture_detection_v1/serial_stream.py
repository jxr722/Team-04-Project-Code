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


def send(conn, result):
    """
    Send posture data to ESP32 as CSV: "score,avg_score\n"

    Args:
        conn: serial.Serial object from connect(), or None.
        result: Result dict from pose.analyze() (with avg_score added).
    """
    if conn is None:
        return

    line = "{},{}\n".format(
        result["score"],
        result["avg_score"],
    )

    try:
        conn.write(line.encode())
    except serial.SerialException:
        print("Serial send failed.")


def disconnect(conn):
    """Close the serial connection."""
    if conn is not None:
        conn.close()
        print("Serial disconnected.")