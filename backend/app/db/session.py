"""Reusable synchronous engine and request-scoped sessions; no schema creation."""

from collections.abc import Generator
from functools import lru_cache

from sqlalchemy import Engine, create_engine
from sqlalchemy.orm import Session, sessionmaker

from app.core.config import get_database_url


@lru_cache(maxsize=1)
def get_engine() -> Engine:
    # Engine construction is lazy: no network connection until the first query.
    return create_engine(
        get_database_url(),
        pool_pre_ping=True,
        pool_size=5,
        max_overflow=5,
        pool_timeout=5,
        connect_args={"connect_timeout": 3},
        hide_parameters=True,
    )


@lru_cache(maxsize=1)
def get_session_factory() -> sessionmaker[Session]:
    return sessionmaker(bind=get_engine())


def get_db() -> Generator[Session, None, None]:
    # close() also rolls back unfinished transactions and returns the connection to the pool.
    with get_session_factory()() as session:
        yield session
