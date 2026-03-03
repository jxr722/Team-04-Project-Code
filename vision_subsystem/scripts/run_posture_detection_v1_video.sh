#!/bin/bash
# Auto-detect USB serial port and run the posture monitor.

PORT=$(ls /dev/tty.usb* 2>/dev/null | head -n 1)

if [ -z "$PORT" ]; then
  echo "No USB serial device found. Running without serial."
  python vision_subsystem/posture_detection_v1/main.py --video "test.mp4"
else
  echo "Found ESP32 at: $PORT"
  python vision_subsystem/posture_detection_v1/main.py --video "test.mp4" --serial-port "$PORT"
fi