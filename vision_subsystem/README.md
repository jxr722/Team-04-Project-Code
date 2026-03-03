# Vision Subsystem Type 1: Posture Monitor with MediaPipe

## Overview
This method utilizes the MediaPipe library to create a real-time posture monitoring system using a webcam or video. It detects body landmarks in 3D, computes neck and torso inclination angles, and derives a posture score (0-100) from the angles.

## Quick Setup and Usage
Make sure you have a fresh `python = 3.10` environment.

``` bash
pip install -r vision_subsystem_v1/requirements.txt
```

- Run with webcam (the model can directly work from the front side)
  ```bash
  bash vision_subsystem/scripts/run_posture_detection_v1_webcam.sh
  ```

- Run with test video
  ```bash
  bash vision_subsystem/scripts/run_posture_detection_v1_video.sh
  ```