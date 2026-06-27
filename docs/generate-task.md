# OxyFeeder: Development Task Breakdown

## Phase 1: Project Setup & System Design

| Task ID | Title | Description | Dependencies | Duration | Role |
|----------|-------|--------------|---------------|----------|------|
| SETUP-01 | Initialize Project Repositories | Create GitHub repos for firmware and app | - | 2h | PM |
| SETUP-02 | Finalize Hardware Bill of Materials | Procure components and verify specs | - | 1d | Systems Architect |
| SETUP-03 | Define Firmware-App API | Specify JSON/MQTT data structure | - | 4h | Embedded & Flutter Dev |

---

## Phase 2: Hardware Integration & Prototyping

| Task ID | Title | Description | Dependencies | Duration | Role |
|----------|-------|--------------|---------------|----------|------|
| HW-01 | Assemble Power System | Wire solar, MPPT, battery, and converters | SETUP-02 | 4h | Embedded Dev |
| HW-02 | Build Feeding Mechanism | Mount DC motor, driver, spinner plate | SETUP-02 | 6h | Embedded Dev |
| HW-03 | Connect Core Sensors | Wire DO, Load Cell, RTC, OLED | SETUP-02 | 8h | Embedded Dev |
| HW-04 | Integrate Arduino and ESP32 | Serial/I2C connection with level shifters | HW-03 | 4h | Embedded Dev |
| HW-05 | Prototype Assembly | Mount and cable management inside enclosure | HW-04 | 1d | Systems Architect |

---

## Phase 3: Firmware Development

| Task ID | Title | Description | Dependencies | Duration | Role |
|----------|-------|--------------|---------------|----------|------|
| FIRM-01 | Sensor Data Reading | Implement DO, Load Cell, RTC reading | HW-03 | 2d | Embedded Dev |
| FIRM-02 | Motor & UI Control | Add feeding logic + OLED updates | FIRM-01 | 1.5d | Embedded Dev |
| FIRM-03 | Control Loop | Combine schedule + alerts logic | FIRM-02 | 2d | Embedded Dev |
| FIRM-04 | WiFi/Bluetooth Comms | ESP32 network setup | HW-04 | 1d | Embedded Dev |
| FIRM-05 | Alerts Handling | Send GSM/App alerts | FIRM-04 | 2d | Embedded Dev |
| FIRM-06 | ESP32-CAM Integration | Capture and send images | FIRM-04 | 1d | Embedded Dev |

---

## Phase 4: Flutter Mobile App Development

| Task ID | Title | Description | Dependencies | Duration | Role |
|----------|-------|--------------|---------------|----------|------|
| APP-01 | UI/UX Wireframing | Design Dashboard, Alerts, Camera View | SETUP-03 | 1d | Flutter Dev |
| APP-02 | Project Setup | Initialize Flutter + WiFi/Bluetooth services | APP-01 | 2d | Flutter Dev |
| APP-03 | Real-Time Dashboard | Live data display (DO, feed, battery) | APP-02 | 2d | Flutter Dev |
| APP-04 | Device Configuration | Manage schedule and thresholds | APP-02 | 1.5d | Flutter Dev |
| APP-05 | Push Notifications | Setup Firebase for alerts | APP-03 | 1d | Flutter Dev |

---

## Phase 5: Testing & QA

| Task ID | Title | Description | Dependencies | Duration | Role |
|----------|-------|--------------|---------------|----------|------|
| TEST-01 | Unit Test Firmware | Test sensors and actuators individually | FIRM-06 | 2d | QA |
| TEST-02 | Integration Testing | Verify system end-to-end | TEST-01 | 2d | QA |
| TEST-03 | E2E Testing | Sensor → firmware → alert → app | TEST-02 | 3d | QA |
| TEST-04 | Field Durability Test | Run 72-hour live simulation | TEST-03 | 3d | QA |
