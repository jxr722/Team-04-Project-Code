# Vision Subsystem Type 1: Posture Monitor with MediaPipe

## Overview
This method utilizes the MediaPipe library to create a real-time posture monitoring system using a webcam or video. It detects body landmarks in 3D, computes neck and torso inclination angles, and derives a posture score (0-100) from the angles.

## Demo
[prototype_v1_demo.mp4](vision_subsystem_v1/vision_subsystem_demo/demo_v1.mp4)

## Quick Setup and Usage
Make sure you have a fresh `python = 3.10` environment.

``` bash
pip install -r requirements.txt
```

- Run with webcam (the model can directly work from the front side)
  ```bash
  python prototype_v1/main.py
  ```

- Run with test video
  ```bash
  python prototype_v1/main.py --video test.mp4
  ```