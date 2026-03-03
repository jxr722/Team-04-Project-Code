import logging
from datetime import datetime

def timestamp():
    """Return the current time as a readable string, e.g. '2026-03-03 14:05:12'."""
    return datetime.now().strftime("%Y-%m-%d %H:%M:%S")

logger = logging.getLogger()

def setup_logging(log_file="posture_monitor.log"):
    """Set up file logging. Overwrites the log file each run."""
    logging.basicConfig(
        filename=log_file,
        filemode="w",
        level=logging.INFO,
        format="%(asctime)s %(levelname)s %(message)s",
    )

def log(result):
    """Log a pose analysis result to the log file."""
    logger.info(
        "score=%d neck_angle=%.1f torso_angle=%.1f world_landmarks=%s",
        result["score"],
        result["neck_angle"],
        result["torso_angle"],
        result["world_landmarks"],
    )
