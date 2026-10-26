# NgậpSense — IoT Water Level Monitoring & Flood Warning System

> **Course project.** A prototype built on a simulated ESP32 and deployed to a cloud server.
> It does not connect to real drainage infrastructure.

> **Current implementation: Step 1 — project scaffolding only.** FastAPI exposes
> `GET /health`; Streamlit shows a placeholder; Docker Compose prepares PostgreSQL.
> The existing firmware and standalone MQTT listener are preserved. The overview,
> requirements, and roadmap below describe planned capabilities.

---

## 1. Overview

**NgậpSense** (from Vietnamese *ngập* — flooding — and *sense*) collects **water level** and **rainfall**
data from sensors, publishes it over **MQTT**, stores it in a database, and displays live and historical
readings on a web dashboard. When readings cross configured thresholds, the system raises alerts on the
dashboard and sends a remote notification. Two extensions are included: a **short-term water level forecast**
and a **servo-driven gate** that models a sluice gate controlled from the dashboard.

The ESP32 and its sensors run in the **Wokwi simulator**, so the system can be demonstrated without hardware.

## 2. Objectives

- Collect water level and rainfall readings and publish them via MQTT.
- Store time-series data and visualize current and historical values.
- Raise alerts against configurable thresholds and notify remotely.
- Forecast water level for a short horizon from past readings.
- Send control commands from the dashboard to the virtual gate.

## 3. Scope

| In scope | Out of scope |
| --- | --- |
| IoT architecture on a simulated test model | City-wide flood forecasting |
| Monitoring, storage, alerting, short-term forecasting | Control of real drainage infrastructure |
| Servo gate simulation for decision support | Integration with municipal SCADA systems |

## 4. Architecture

```mermaid
flowchart LR
  subgraph Edge[Wokwi simulation]
    S1[Water level sensor] --> ESP[ESP32]
    S2[Rain sensor] --> ESP
    ESP --> ACT[Servo gate]
  end
  ESP <-->|MQTT| B[(MQTT broker)]
  B <--> BE[Backend API]
  BE <--> DB[(Database)]
  BE --> ML[Forecast service]
  BE <--> FE[Streamlit dashboard]
  BE --> NOTI[Notifications]
  USER[User] --> FE
```

- **Monitoring:** `Sensor → ESP32 → MQTT broker → Backend → Database → Dashboard`
- **Control:** `Dashboard → Backend → MQTT broker → ESP32 → Servo`
- Stored readings feed the forecast service; predictions are shown alongside live measurements.

## 5. Components

| Layer | Technology | Role |
| --- | --- | --- |
| Edge | ESP32, potentiometer (level), rain sensor, servo — Wokwi | Measure and publish data; receive commands |
| Transport | HiveMQ Cloud (or local Mosquitto) | Message relay between device and server |
| Backend | FastAPI, paho-mqtt, SQLAlchemy | Process and store data, evaluate thresholds, issue commands |
| Database | PostgreSQL | Compose infrastructure in Step 1; application storage planned |
| Analytics | scikit-learn | Short-horizon water level forecast |
| Dashboard | Streamlit, Plotly | Live view, history, alerts, controls |
| Hosting | Render (two Python web services) | Public deployment of backend and dashboard |

## 6. Functional Requirements & Progress

**Status:** ✅ Done · 🚧 In progress · ⬜ Not started · 🔶 Extension

| ID | Feature | Description | Priority | Status |
| --- | --- | --- | --- | --- |
| FR01 | Water level monitoring | Read level via sensor and ESP32, publish over MQTT | Core | ⬜ |
| FR02 | Rainfall tracking | Read rain sensor, process and publish measurements | Core | ⬜ |
| FR03 | Data display | Show current level, rainfall, and system status | Core | ⬜ |
| FR04 | Data storage | Persist time-series readings in the database | Core | ⬜ |
| FR05 | Historical visualization | Query and chart readings over a selected time range | Core | ⬜ |
| FR06 | Flood risk alert | Compare level with thresholds; alert on dashboard and/or device | Core | ⬜ |
| FR07 | Remote notification | Email or push message when an alert condition is met | Core | ⬜ |
| FR08 | Water level forecast | Short-horizon forecast from historical data | Extension | 🔶 |
| FR09 | Drainage control support | Send MQTT commands to open/close the simulated gate | Extension | 🔶 |
| FR10 | Device status monitoring | Show connection and health status of main components | Optional | ⬜ |
| FR11 | Access management | Register, log in, authenticate protected actions | Optional | ⬜ |
| FR12 | System information query | Look up current level, history, and alert state | Optional | ⬜ |

**Functional requirements progress:** `0%` — _updated after each milestone._

FR01–FR07 are the core monitoring and alerting scope; FR08–FR09 are extensions; FR10–FR12 depend on time.

## 7. Target Specifications

| Item | Target |
| --- | --- |
| Sampling interval | 5–10 s (configurable) |
| Update latency | ≤ 3 s from simulator to dashboard |
| Alert thresholds | 0.3 m watch, 0.5 m warning, 0.8 m danger |
| Retention | At least 30 days of readings |
| Forecast horizon | 15–60 min, evaluated with MAE/RMSE |
| Connectivity loss | Device auto-reconnects to the broker |

## 8. MQTT Topics

| Topic | Direction | Payload |
| --- | --- | --- |
| `ngapsense/{device_id}/telemetry` | Device → Server | `{ "ts", "water_level", "rain", "rssi" }` |
| `ngapsense/{device_id}/status` | Device → Server | `{ "online", "fw", "uptime" }` |
| `ngapsense/{device_id}/alert` | Device → Server | `{ "level", "value" }` |
| `ngapsense/{device_id}/cmd` | Server → Device | `{ "action": "open" \| "close", "gate_id" }` |
| `ngapsense/{device_id}/cmd/ack` | Device → Server | `{ "action", "result" }` |

QoS: telemetry `0/1`, commands `1`, status `1` with a last-will message.

## 9. Repository Layout

```
ngapsense/
├── firmware/              # Existing ESP32 firmware, unchanged
├── backend/
│   ├── app/
│   │   ├── __init__.py
│   │   ├── main.py        # FastAPI application and /health
│   │   ├── api/
│   │   ├── core/
│   │   ├── db/
│   │   ├── models/
│   │   ├── schemas/
│   │   └── services/
│   ├── mqtt_listener.py   # Existing standalone listener, unchanged
│   └── .env.example       # Existing MQTT configuration example
├── frontend/
│   ├── app.py             # Streamlit placeholder
│   ├── pages/
│   ├── components/
│   └── services/
├── docker-compose.yml     # PostgreSQL only
├── .env.example           # PostgreSQL configuration
├── .python-version
├── pyproject.toml
├── uv.lock
├── main.py                # Original uv starter script, unchanged
└── README.md
```

Backend subpackages and frontend components/services contain empty `__init__.py`
files only. `frontend/pages/.gitkeep` keeps the empty directory in Git without
adding a page to Streamlit. Other existing empty/cache directories are omitted.
No ORM models, database tables, migrations, or seed scripts are added.

## 10. Running Locally (Step 1)

Run commands from the repository root. Prerequisites: Python 3.13+ and
[uv](https://docs.astral.sh/uv/). Docker with Docker Compose v2 is needed only
for PostgreSQL. FastAPI and Streamlit run independently without a database or broker.

Install dependencies from the lockfile:

```powershell
uv sync --locked
```

Start FastAPI in terminal 1:

```powershell
uv run uvicorn app.main:app --app-dir backend --reload --port 8000
```

Open <http://127.0.0.1:8000/health> (HTTP 200, `{"status":"ok"}`), or run:

```powershell
Invoke-RestMethod http://127.0.0.1:8000/health
```

API documentation is at <http://127.0.0.1:8000/docs>. `/health` checks only that
the API is responding; it does not check PostgreSQL or MQTT.

Start Streamlit in terminal 2:

```powershell
uv run streamlit run frontend/app.py --server.port 8501
```

Open <http://localhost:8501> to see the NgậpSense title and placeholder message.
There are no charts, simulated readings, or API calls.

For PostgreSQL, copy the example once (keep any existing `.env`), then edit
`.env` to choose `POSTGRES_PASSWORD`:

```powershell
Copy-Item .env.example .env
# Edit .env before continuing.
docker compose config --quiet
docker compose up -d --wait postgres
docker compose ps
docker compose exec postgres pg_isready -U ngapsense -d ngapsense
```

The last command assumes the example user/database names; substitute your values
if changed. `POSTGRES_PORT` selects the local port (default `5432`), bound to
`127.0.0.1`. PostgreSQL 17 uses the named `postgres_data` volume and has a readiness
healthcheck. Stop it while retaining data with:

```powershell
docker compose down
```

The image initializes the database/user on its first start with an empty volume.
Changing `.env` later does not change credentials in an existing volume.
No application tables are created. FastAPI does not connect to PostgreSQL.

The existing MQTT listener still loads `backend/.env`, independently of the root
`.env`. To run that existing tool separately (optional):

```powershell
Copy-Item backend/.env.example backend/.env  # First setup only; keep existing settings.
uv run python backend/mqtt_listener.py
```

### Step 1 verification (2026-10-26)

- `uv sync --locked`, `uv lock --check`, and `git diff --check` passed.
- A real Uvicorn process started; `/health` returned HTTP 200 and `{"status":"ok"}`.
  OpenAPI listed `/health` as the only application endpoint.
- A real Streamlit process started; its health endpoint and HTML shell returned
  HTTP 200. Streamlit AppTest confirmed the title, description, and placeholder
  without exceptions. Browser visual inspection was unavailable in this environment.
- YAML syntax and the Compose environment/volume/healthcheck structure were checked.
  Docker was unavailable, so `docker compose config` and PostgreSQL startup/readiness
  were not verified at runtime.
- SHA-256 comparison confirmed all tracked firmware files, `backend/mqtt_listener.py`,
  and `backend/.env.example` were unchanged. ESP32 simulation and broker connectivity
  were not rerun. Temporary verification servers were stopped.

## 11. Future Work (not implemented in Step 1)

Database connections, ORM models, migrations, sensor/device APIs, dashboard
charts, authentication, warnings, MQTT integration with FastAPI, servo controls,
Machine Learning, and cloud deployment are not implemented by this scaffold.
The existing firmware/listener keep their current behavior.

## 12. Roadmap

| Phase | Work | Deliverable |
| --- | --- | --- |
| M1 | Broker setup, Wokwi diagram, pin map | Working simulation |
| M2 | Sketch: sensor reads and MQTT publish | Telemetry stream |
| M3 | Backend, database, dashboard | FR01–FR05 |
| M4 | Threshold alerts and notifications | FR06–FR07 |
| M5 | Gate control and device status | FR09–FR10 |
| M6 | Forecast model and evaluation | FR08, results |
| M7 | Deploy, test, demo, report | Final submission |

## 13. Limitations

- Simulated sensing: readings come from virtual inputs, so real sensor error is not measured.
- Free hosting tiers sleep when idle, which drops the MQTT connection until the next request.
- Forecasts are indicative only and not suitable for operational use.
- Gate control is a demonstration of the decision loop, not real actuation.
- Login state does not persist across page reloads in the dashboard.
