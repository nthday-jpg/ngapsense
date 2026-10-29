from unittest.mock import Mock

import psycopg
import pytest
from sqlalchemy import create_engine, event, text
from sqlalchemy.orm import sessionmaker
from sqlalchemy.pool import QueuePool

from app.db import session


def test_engine_and_factory_reused_without_opening_connections(monkeypatch):
    connect = Mock(side_effect=AssertionError("Unexpected network connection"))
    monkeypatch.setattr(psycopg, "connect", connect)
    engine = session.get_engine()
    factory = session.get_session_factory()
    assert session.get_engine() is engine
    assert session.get_session_factory() is factory
    assert engine.pool._pre_ping is True
    with factory() as first, factory() as second:
        assert first is not second
    connect.assert_not_called()


@pytest.mark.parametrize("with_error", [False, True])
def test_dependency_releases_connection_and_rolls_back(monkeypatch, with_error):
    # SQLite verifies SQLAlchemy session cleanup independently of PostgreSQL.
    # No tables or application data are created.
    engine = create_engine("sqlite://", poolclass=QueuePool)
    rollback = Mock()
    event.listen(engine, "rollback", rollback)
    monkeypatch.setattr(session, "get_session_factory", lambda: sessionmaker(bind=engine))
    dependency = session.get_db()
    try:
        db = next(dependency)
        assert db.execute(text("SELECT 1")).scalar_one() == 1
        assert engine.pool.checkedout() == 1
        if with_error:
            with pytest.raises(RuntimeError, match="request failed"):
                dependency.throw(RuntimeError("request failed"))
        else:
            with pytest.raises(StopIteration):
                next(dependency)
        assert engine.pool.checkedout() == 0
        rollback.assert_called_once()
    finally:
        dependency.close()
        engine.dispose()
