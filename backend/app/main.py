from contextlib import asynccontextmanager
from typing import Annotated

from fastapi import Depends, FastAPI
from fastapi.responses import JSONResponse
from sqlalchemy import text
from sqlalchemy.exc import SQLAlchemyError
from sqlalchemy.orm import Session

from app.db.session import get_db, get_engine


@asynccontextmanager
async def lifespan(app: FastAPI):
    # Validate configuration at startup, without requiring PostgreSQL to be online.
    engine = get_engine()
    try:
        yield
    finally:
        engine.dispose()


app = FastAPI(title="NgậpSense", lifespan=lifespan)


@app.get("/health")
async def health() -> dict[str, str]:
    return {"status": "ok"}


@app.get("/health/db")
def database_health(db: Annotated[Session, Depends(get_db)]) -> JSONResponse:
    try:
        db.execute(text("SELECT 1")).scalar_one()
    except SQLAlchemyError:
        return JSONResponse(status_code=503, content={"database": "unavailable"})
    return JSONResponse(content={"database": "connected"})
