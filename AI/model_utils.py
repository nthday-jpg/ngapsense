"""Utilities for saving and loading the trained flood forecasting model."""

from pathlib import Path

import joblib

MODEL_PATH = Path(__file__).with_name("random_forest_flood_model.joblib")


def save_model(model, path=MODEL_PATH):
    """Save the trained model and its feature order to a joblib file."""
    path = Path(path)
    bundle = {
        "model": model,
        "feature_columns": list(model.feature_names_in_),
    }
    joblib.dump(bundle, path)
    return path


def load_model(path=MODEL_PATH):
    """Load a model bundle saved by save_model()."""
    path = Path(path)
    if not path.exists():
        raise FileNotFoundError(f"Model file not found: {path}")
    bundle = joblib.load(path)
    return bundle["model"], bundle["feature_columns"]
