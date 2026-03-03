import math
import cv2
import mediapipe as mp
from utils import timestamp


def init():
    """Create and return a MediaPipe Pose detector."""
    return mp.solutions.pose.Pose()


def analyze(pose, frame, neck_limit, torso_limit, visibility_factor=1):
    """
    Analyze a single frame for posture.

    Args:
        pose: MediaPipe Pose object from init().
        frame: BGR image.
        neck_limit: Used for scoring. (degrees).
        torso_limit: Used for scoring. (degrees).

    Returns:
        A result dict.
    """
    h, w = frame.shape[:2]

    rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    keypoints = pose.process(rgb)

    lm = keypoints.pose_landmarks
    if lm is None:
        return None

    lm_enum = mp.solutions.pose.PoseLandmark

    # Extract all 6 landmark pixel coordinates (for display).
    landmarks = {
        "l_shoulder": (int(lm.landmark[lm_enum.LEFT_SHOULDER].x * w),
                       int(lm.landmark[lm_enum.LEFT_SHOULDER].y * h)),
        "r_shoulder": (int(lm.landmark[lm_enum.RIGHT_SHOULDER].x * w),
                       int(lm.landmark[lm_enum.RIGHT_SHOULDER].y * h)),
        "l_ear":      (int(lm.landmark[lm_enum.LEFT_EAR].x * w),
                       int(lm.landmark[lm_enum.LEFT_EAR].y * h)),
        "r_ear":      (int(lm.landmark[lm_enum.RIGHT_EAR].x * w),
                       int(lm.landmark[lm_enum.RIGHT_EAR].y * h)),
        "l_hip":      (int(lm.landmark[lm_enum.LEFT_HIP].x * w),
                       int(lm.landmark[lm_enum.LEFT_HIP].y * h)),
        "r_hip":      (int(lm.landmark[lm_enum.RIGHT_HIP].x * w),
                       int(lm.landmark[lm_enum.RIGHT_HIP].y * h)),
    }

    # Use world landmarks for camera-independent angle computation.
    wlm = keypoints.pose_world_landmarks
    if wlm is None:
        return None

    # Extract 3D world coordinates for both sides.
    l_shoulder_3d = (wlm.landmark[lm_enum.LEFT_SHOULDER].x,
                     wlm.landmark[lm_enum.LEFT_SHOULDER].y,
                     wlm.landmark[lm_enum.LEFT_SHOULDER].z)
    r_shoulder_3d = (wlm.landmark[lm_enum.RIGHT_SHOULDER].x,
                     wlm.landmark[lm_enum.RIGHT_SHOULDER].y,
                     wlm.landmark[lm_enum.RIGHT_SHOULDER].z)
    l_ear_3d = (wlm.landmark[lm_enum.LEFT_EAR].x,
                wlm.landmark[lm_enum.LEFT_EAR].y,
                wlm.landmark[lm_enum.LEFT_EAR].z)
    r_ear_3d = (wlm.landmark[lm_enum.RIGHT_EAR].x,
                wlm.landmark[lm_enum.RIGHT_EAR].y,
                wlm.landmark[lm_enum.RIGHT_EAR].z)
    l_hip_3d = (wlm.landmark[lm_enum.LEFT_HIP].x,
                wlm.landmark[lm_enum.LEFT_HIP].y,
                wlm.landmark[lm_enum.LEFT_HIP].z)
    r_hip_3d = (wlm.landmark[lm_enum.RIGHT_HIP].x,
                wlm.landmark[lm_enum.RIGHT_HIP].y,
                wlm.landmark[lm_enum.RIGHT_HIP].z)

    world_landmarks = {
            "l_shoulder": l_shoulder_3d,
            "r_shoulder": r_shoulder_3d,
            "l_ear": l_ear_3d,
            "r_ear": r_ear_3d,
            "l_hip": l_hip_3d,
            "r_hip": r_hip_3d,
        }

    # Project both sides to sagittal plane.
    l_proj = project_to_sagittal(
        l_shoulder_3d, r_shoulder_3d,
        l_shoulder_3d, l_ear_3d, l_hip_3d,
    )
    r_proj = project_to_sagittal(
        l_shoulder_3d, r_shoulder_3d,
        r_shoulder_3d, r_ear_3d, r_hip_3d,
    )

    # Compute angles for left side.
    l_shoulder_2d, l_ear_2d, l_hip_2d = l_proj["shoulder"], l_proj["ear"], l_proj["hip"]
    l_neck_ang = abs(math.degrees(math.atan2(l_ear_2d[0] - l_shoulder_2d[0], l_ear_2d[1] - l_shoulder_2d[1])))
    l_torso_ang = abs(math.degrees(math.atan2(l_shoulder_2d[0] - l_hip_2d[0], l_shoulder_2d[1] - l_hip_2d[1])))

    # Compute angles for right side.
    r_shoulder_2d, r_ear_2d, r_hip_2d = r_proj["shoulder"], r_proj["ear"], r_proj["hip"]
    r_neck_ang = abs(math.degrees(math.atan2(r_ear_2d[0] - r_shoulder_2d[0], r_ear_2d[1] - r_shoulder_2d[1])))
    r_torso_ang = abs(math.degrees(math.atan2(r_shoulder_2d[0] - r_hip_2d[0], r_shoulder_2d[1] - r_hip_2d[1])))

    # Visibility weighted
    # visibility_factor=1: linear,
    # visibility_factor>1: less visible side's impact fades faster.
    # visibility_factor<1: less visible side's impact fades slower.
    # visibility_factor=0: less visible side's impact does not fade.
    l_vis = lm.landmark[lm_enum.LEFT_EAR].visibility ** visibility_factor
    r_vis = lm.landmark[lm_enum.RIGHT_EAR].visibility ** visibility_factor
    total_vis = l_vis + r_vis
    if total_vis == 0:
        l_w, r_w = 0.5, 0.5
    else:
        l_w = l_vis / total_vis
        r_w = r_vis / total_vis

    neck_angle = l_w * l_neck_ang + r_w * r_neck_ang
    torso_angle = l_w * l_torso_ang + r_w * r_torso_ang

    # Score: 0-100 (100 = perfect upright).
    # Will delve into this more later
    neck_score = max(0.0, (1.0 - neck_angle / (2 * neck_limit))) * 80
    torso_score = max(0.0, (1.0 - torso_angle / (2 * torso_limit))) * 20
    score = int(neck_score + torso_score)

    return {
        "timestamp": timestamp(),
        "landmarks": landmarks,
        "world_landmarks": world_landmarks,
        "neck_angle": neck_angle,
        "torso_angle": torso_angle,
        "score": score,
    }


def project_to_sagittal(l_shoulder_3d, r_shoulder_3d,
                        near_shoulder_3d, near_ear_3d, near_hip_3d):
    """
    Project near-side body points onto the sagittal (side-view) 2D plane.
    Removes the body's horizontal rotation so angles are camera-independent.

    Args:
        l_shoulder_3d: (x, y, z) left shoulder in world coordinates (meters).
        r_shoulder_3d: (x, y, z) right shoulder in world coordinates (meters).
        near_shoulder_3d: (x, y, z) shoulder on the visible side.
        near_ear_3d: (x, y, z) ear on the visible side.
        near_hip_3d: (x, y, z) hip on the visible side.

    Returns:
        Dict with "neck_angle" and "torso_angle" in degrees.
    """
    # Body's horizontal rotation angle from the shoulder vector (x-z plane).
    lateral_x = r_shoulder_3d[0] - l_shoulder_3d[0]
    lateral_z = r_shoulder_3d[2] - l_shoulder_3d[2]
    body_angle = math.atan2(lateral_z, lateral_x)

    # Un-rotate each point to cancel body rotation.
    cos_a = math.cos(-body_angle)
    sin_a = math.sin(-body_angle)

    def unrotate(p):
        """Rotate point in x-z plane to align body to canonical side view."""
        x_new = p[0] * cos_a + p[2] * sin_a
        z_new = -p[0] * sin_a + p[2] * cos_a
        y = p[1]  # vertical axis unchanged
        return (x_new, y, z_new)

    shoulder = unrotate(near_shoulder_3d)
    ear = unrotate(near_ear_3d)
    hip = unrotate(near_hip_3d)

    # Return 2D projections
    return {
        "shoulder": (shoulder[2], -shoulder[1]),
        "ear":      (ear[2], -ear[1]),
        "hip":      (hip[2], -hip[1]),
    }


def _distance(p1, p2):
    """Euclidean distance between two (x, y) points."""
    return math.sqrt((p2[0] - p1[0])**2 + (p2[1] - p1[1])**2)


def _angle_3pt(a, b, c):
    """
    Angle at vertex b formed by points a-b-c, in degrees.
    Returns 180 if points are collinear (straight line), 0 if folded.

    Args:
        a, b, c: 2D points as (x, y) tuples.
    """
    ba = (a[0] - b[0], a[1] - b[1])
    bc = (c[0] - b[0], c[1] - b[1])
    dot = ba[0] * bc[0] + ba[1] * bc[1]
    mag_ba = math.sqrt(ba[0]**2 + ba[1]**2)
    mag_bc = math.sqrt(bc[0]**2 + bc[1]**2)
    if mag_ba * mag_bc == 0:
        return 180.0
    cos_val = max(-1.0, min(1.0, dot / (mag_ba * mag_bc)))
    return math.degrees(math.acos(cos_val))