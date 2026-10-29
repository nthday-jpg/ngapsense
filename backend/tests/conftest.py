import pytest

from app.core import config
from app.db import session


@pytest.fixture(autouse=True)
def isolated_database_config(monkeypatch, tmp_path):
    engine_getter = session.get_engine
    factory_getter = session.get_session_factory
    # Tests never use a developer's real .env or database credentials.
    monkeypatch.setattr(config, "ENV_FILE", tmp_path / ".env")
    monkeypatch.setenv(
        "DATABASE_URL", "postgresql+psycopg://test:unit-test-secret@127.0.0.1:5432/test"
    )
    yield
    if engine_getter.cache_info().currsize:
        engine_getter().dispose()
    factory_getter.cache_clear()
    engine_getter.cache_clear()
