"""Database configuration from the process environment or the root .env file."""

import os
from pathlib import Path

from dotenv import dotenv_values
from sqlalchemy.engine import URL, make_url
from sqlalchemy.exc import ArgumentError

ENV_FILE = Path(__file__).resolve().parents[3] / ".env"


def get_database_url() -> URL:
    value = os.environ.get("DATABASE_URL")
    if value is None:
        value = dotenv_values(ENV_FILE).get("DATABASE_URL")
    if not value or not value.strip():
        raise RuntimeError("DATABASE_URL is required. Set it in the environment or root .env file.")

    try:
        url = make_url(value.strip())
        valid = (
            url.drivername == "postgresql+psycopg"
            and url.host
            and url.username
            and url.database
            and (url.port is None or 1 <= url.port <= 65535)
        )
    except (ArgumentError, ValueError):
        valid = False

    if not valid:
        # Never include the supplied URL or the parser's exception: both may contain secrets.
        raise RuntimeError(
            "DATABASE_URL must be a valid postgresql+psycopg URL with a host, user and database."
        ) from None
    return url
