"""
Train and evaluate a Random Forest model for 30-minute water level forecasting.

The model is trained on the synthetic data produced by generate_flood_dataset.py,
evaluated with a chronological 80/20 split, then saved for later inference.

Run after generating the dataset:
    python train_random_forest.py

Dependencies: numpy, pandas, scikit-learn, joblib
"""

from pathlib import Path

import numpy as np
import pandas as pd
from sklearn.ensemble import RandomForestRegressor
from sklearn.metrics import mean_absolute_error, mean_squared_error

from model_utils import load_model, save_model

DATA_PATH = Path(__file__).with_name("flood_dataset.csv")
RANDOM_SEED = 42
TEST_RATIO = 0.2

FEATURE_COLS = [
    "water_level_cm",
    "water_level_lag_5m",
    "water_level_lag_15m",
    "water_level_lag_30m",
    "water_level_change_5m",
    "rainfall_mm_5m",
    "rainfall_mm_15m",
    "rainfall_mm_60m",
    "tide_level_cm",
]


def evaluate(name, y_true, y_pred):
    """Print MAE and RMSE for one prediction method."""
    mae = mean_absolute_error(y_true, y_pred)
    rmse = np.sqrt(mean_squared_error(y_true, y_pred))
    print(f"{name:<28} MAE = {mae:.3f} cm | RMSE = {rmse:.3f} cm")
    return mae, rmse


def main():
    if not DATA_PATH.exists():
        raise FileNotFoundError(
            f"Dataset not found: {DATA_PATH}. "
            "Run generate_flood_dataset.py first."
        )

    df = pd.read_csv(DATA_PATH, parse_dates=["timestamp"])
    if not df["timestamp"].is_monotonic_increasing:
        raise ValueError("Dataset must be sorted in chronological order.")

    X = df[FEATURE_COLS].copy()
    X["drainage_restricted"] = (df["drainage_status"] == "restricted").astype(int)
    y = df["water_level_future_30m"]

    # Chronological split: first 80% for training, last 20% for testing.
    split = int(len(df) * (1 - TEST_RATIO))
    X_train, X_test = X.iloc[:split], X.iloc[split:]
    y_train, y_test = y.iloc[:split], y.iloc[split:]

    model = RandomForestRegressor(
        n_estimators=200,
        min_samples_leaf=3,
        random_state=RANDOM_SEED,
        n_jobs=-1,
    )
    model.fit(X_train, y_train)
    pred_rf = model.predict(X_test)

    # Baseline: predict the water level 30 minutes ahead using the current level.
    pred_baseline = X_test["water_level_cm"].to_numpy()

    print("=" * 70)
    print("KẾT QUẢ DỰ BÁO (80% đầu train, 20% cuối test)")
    print("=" * 70)
    print(f"Train: {len(X_train)} dòng | Test: {len(X_test)} dòng")
    mae_rf, _ = evaluate("Random Forest", y_test, pred_rf)
    mae_base, _ = evaluate("Baseline (mực nước hiện tại)", y_test, pred_baseline)
    print(f"\nMAE giảm so với baseline: {(1 - mae_rf / mae_base) * 100:.1f}%")

    importance = pd.Series(model.feature_importances_, index=X.columns)
    print("\nMức độ quan trọng của các feature (top 5):")
    print(importance.sort_values(ascending=False).head(5).round(3).to_string())

    # Save the trained model, then load it back to verify the artifact.
    model_path = save_model(model)
    loaded_model, loaded_features = load_model(model_path)
    if loaded_features != list(X.columns):
        raise ValueError("Saved feature order does not match the training data.")

    loaded_pred = loaded_model.predict(X_test[loaded_features])
    max_diff = np.max(np.abs(loaded_pred - pred_rf))

    print(f"\nĐã lưu mô hình: {model_path}")
    print(f"Đã load lại mô hình: {len(loaded_features)} features")
    print(f"Chênh lệch dự đoán lớn nhất sau load: {max_diff:.8f} cm")


if __name__ == "__main__":
    main()
