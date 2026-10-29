import socket
from unittest.mock import Mock

import psycopg
import pytest
from fastapi.testclient import TestClient
from sqlalchemy.exc import OperationalError, TimeoutError as PoolTimeoutError
from sqlalchemy.orm import Session

from app.db.session import get_db, get_engine
from app.main import app


def test_liveness_does_not_need_a_connection_and_shutdown_disposes_pool(monkeypatch):
    connect = Mock(side_effect=AssertionError("Unexpected network connection"))
    monkeypatch.setattr(psycopg, "connect", connect)
    engine = get_engine()
    dispose = Mock(wraps=engine.dispose)
    monkeypatch.setattr(engine, "dispose", dispose)
    with TestClient(app) as client:
        response = client.get("/health")
        assert response.status_code == 200
        assert response.json() == {"status": "ok"}
        assert set(client.get("/openapi.json").json()["paths"]) == {"/health", "/health/db"}
    connect.assert_not_called()
    dispose.assert_called_once()


def test_missing_config_fails_startup_clearly(monkeypatch):
    monkeypatch.delenv("DATABASE_URL")
    with pytest.raises(RuntimeError, match="DATABASE_URL is required"):
        with TestClient(app):
            pass


def test_db_endpoint_success_failure_and_recovery_without_api_restart():
    db = Mock(spec=Session)

    def override_db():
        yield db

    app.dependency_overrides[get_db] = override_db
    try:
        with TestClient(app) as client:
            assert client.get("/health/db").json() == {"database": "connected"}
            assert client.get("/health/db").status_code == 200
            assert str(db.execute.call_args.args[0]) == "SELECT 1"
            db.execute.side_effect = OperationalError(
                "SELECT 1", {}, Exception("postgresql://test:unit-test-secret@localhost/test")
            )
            response = client.get("/health/db")
            assert response.status_code == 503
            assert response.json() == {"database": "unavailable"}
            assert "unit-test-secret" not in response.text
            assert client.get("/health").json() == {"status": "ok"}
            db.execute.side_effect = None
            assert client.get("/health/db").status_code == 200
    finally:
        app.dependency_overrides.clear()


def test_pool_timeout_returns_503():
    db = Mock(spec=Session)
    db.execute.side_effect = PoolTimeoutError("pool timeout")

    def override_db():
        yield db

    app.dependency_overrides[get_db] = override_db
    try:
        with TestClient(app) as client:
            response = client.get("/health/db")
            assert response.status_code == 503
            assert response.json() == {"database": "unavailable"}
    finally:
        app.dependency_overrides.clear()


def test_real_driver_connection_refused_keeps_api_alive(monkeypatch, caplog):
    # Reserve a local port without listening so no running database can be contacted.
    with socket.socket() as reserved:
        reserved.bind(("127.0.0.1", 0))
        port = reserved.getsockname()[1]
        monkeypatch.setenv(
            "DATABASE_URL",
            f"postgresql+psycopg://test:unit-test-secret@127.0.0.1:{port}/test",
        )
        with TestClient(app) as client:
            for _ in range(2):
                response = client.get("/health/db")
                assert response.status_code == 503
                assert response.json() == {"database": "unavailable"}
                assert client.get("/health").status_code == 200
            assert get_engine().pool.checkedout() == 0
    assert "unit-test-secret" not in caplog.text
