import pytest

from app.core import config


def test_process_environment_takes_priority(monkeypatch):
    config.ENV_FILE.write_text(
        "DATABASE_URL=postgresql+psycopg://file:password@localhost/file_db\n",
        encoding="utf-8",
    )
    monkeypatch.setenv("DATABASE_URL", "postgresql+psycopg://env:password@localhost/env_db")
    assert config.get_database_url().database == "env_db"


def test_root_env_is_loaded_without_changing_process_environment(monkeypatch):
    monkeypatch.delenv("DATABASE_URL")
    config.ENV_FILE.write_text(
        "DATABASE_URL=postgresql+psycopg://file:p%40ss@localhost:5433/file_db\n",
        encoding="utf-8",
    )
    url = config.get_database_url()
    assert (url.database, url.port, url.password) == ("file_db", 5433, "p@ss")
    assert "DATABASE_URL" not in config.os.environ


def test_missing_configuration_has_clear_error(monkeypatch):
    monkeypatch.delenv("DATABASE_URL")
    with pytest.raises(RuntimeError, match="DATABASE_URL is required"):
        config.get_database_url()


@pytest.mark.parametrize(
    "value",
    [
        "",
        "   ",
        "unit-test-secret",
        "postgresql+psycopg://test:unit-test-secret@localhost:bad/test",
        "postgresql+psycopg://test:unit-test-secret@localhost:65536/test",
        "postgresql+psycopg://test:unit-test-secret@/test",
        "postgresql+psycopg://test:unit-test-secret@localhost",
        "sqlite:///unit-test-secret",
        "postgresql://test:unit-test-secret@localhost/test",
    ],
)
def test_invalid_configuration_does_not_expose_url(monkeypatch, value):
    monkeypatch.setenv("DATABASE_URL", value)
    with pytest.raises(RuntimeError, match="DATABASE_URL") as error:
        config.get_database_url()
    assert "unit-test-secret" not in str(error.value)


def test_empty_environment_does_not_fall_back_to_file(monkeypatch):
    config.ENV_FILE.write_text(
        "DATABASE_URL=postgresql+psycopg://file:password@localhost/file_db\n",
        encoding="utf-8",
    )
    monkeypatch.setenv("DATABASE_URL", "")
    with pytest.raises(RuntimeError, match="DATABASE_URL is required"):
        config.get_database_url()
