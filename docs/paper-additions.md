# OxyFeeder — IEEE Paper Additions & Corrections

Draft sections and corrections to add to the IEEE paper. Copy-paste each block into the appropriate location in your document.

---

## CORRECTION 1: Fix the Limitations Paragraph (Page 2)

**Current (WRONG — contradicts the mobile app):**
> "Furthermore, no control-based interaction, no mapping or transmission occurs with the prototype. The only connection between the device and hand retrieval using a rope."

**Replace with:**
> The prototype supports remote monitoring and control through a mobile application via Bluetooth, but it has no self-mobility — the floating unit is repositioned or retrieved manually using a rope. While this is a low-cost solution that works well, future iterations should be remote-controlled or self-stabilizing for improved access and autonomous positioning.

---

## CORRECTION 2: Replace "pellet" Wording

Three small wording fixes (load cell leftovers):

| Location | Current | Change To |
|----------|---------|-----------|
| SIM800L description | "oxygen levels, and pellet inventory" | "oxygen levels, and feed level" |
| TFT LCD description | "battery status, and pellet percentage" | "battery status, and feed level percentage" |
| Software Dev intro | "battery charge, and pellet consumption" | "battery charge, and feed level" |

---

## EXPANDED SECTION: C. Software Development

**Replace the current thin Software Development paragraph with this expanded version:**

The software consists of three components: the Arduino firmware, the ESP32 firmware, and the mobile application.

The Arduino firmware continuously acquires real-time data including dissolved oxygen concentration, feed level, and battery voltage. These readings are validated, displayed on the TFT LCD, and transmitted as JSON to the ESP32. The firmware also handles motor control for feed dispensing, RTC-based scheduling, and SMS alert logic through the SIM800L module.

The ESP32 firmware acts as the communication bridge, broadcasting sensor data over Bluetooth Low Energy (BLE) and relaying commands from the mobile application to the Arduino. It also manages feeding schedules and threshold values synced from the app.

The mobile application, developed using Flutter, provides the primary user interface (Figures 4 and 5). It features a real-time dashboard showing dissolved oxygen, feed level, and battery status; a settings panel for configuring feeding schedules and safety thresholds; manual feed control; and a live camera view streamed from the ESP32-CAM. The application connects automatically via Bluetooth, allowing farmers to monitor and control the system remotely. The system acquires real-time data, validates and displays it, and notifies the user with automated feeding alerts and status updates when feeding is complete.

---

## NEW SUBSECTION: D. System Architecture and Communication

**Add this after the Components and Materials section (II-B):**

The system follows a layered communication architecture. The Arduino Mega 2560 acts as the central controller, continuously reading data from the dissolved oxygen sensor, ultrasonic sensor, and voltage sensor. Sensor data is formatted as JSON and transmitted to the ESP32 microcontroller through a serial connection. The ESP32 functions as a Bluetooth Low Energy (BLE) bridge, broadcasting the data to the mobile application and relaying user commands back to the Arduino. Commands such as manual feeding and threshold updates are sent from the app to the ESP32, which signals the Arduino to execute the corresponding action. The ESP32-CAM operates independently, streaming live video over WiFi. This separation allows sensor monitoring through Bluetooth and video streaming through WiFi to function simultaneously without interference.

---

## NEW SECTION: III. Results and Discussion

**Add this entire section after Software Development:**

### III. RESULTS AND DISCUSSION

The prototype was assembled and tested to evaluate its functionality, reliability, and performance under simulated fishpond conditions. Each subsystem was verified individually before full integration.

**A. Feeding Mechanism**

The automated feeding mechanism successfully dispensed feed both on schedule and through manual triggering from the mobile application. The servo gate opened for approximately 0.15 seconds while the DC motor distributed the feed evenly. Scheduled feedings executed reliably based on the RTC module, even when the mobile device was disconnected, confirming that schedules are stored and processed on the device itself.

**B. Sensor Monitoring**

The dissolved oxygen, feed level, and battery sensors provided continuous real-time readings displayed on both the TFT LCD and the mobile application. The ultrasonic sensor measured feed level by distance to the feed surface, applying a median filter to reduce erroneous readings and improve stability. [INSERT: your observations on reading accuracy and stability.]

**C. Alert System**

The SMS alert system successfully notified the user of low feed and low battery conditions through the SIM800L module, operating independently of Bluetooth connectivity. A cooldown interval was implemented to prevent message spam. A local buzzer provided audible alerts directly at the device as a backup notification layer.

**D. Power System**

The solar panel and MPPT charge controller maintained battery charge during daylight, supporting sustainable off-grid operation. [INSERT: your observations on battery runtime and charging performance.]

**E. Evaluation Results**

The system was evaluated using the Beta Testing Criteria across eight categories: product quality, functional testing, functionality, reliability, usability, efficiency, maintainability, and portability. The prototype achieved an overall score of [INSERT SCORE]%, indicating satisfactory performance and readiness for real-world deployment.

[INSERT TABLE: Beta testing scores per category — see template below.]

**Table 1. Beta Testing Evaluation Results**

| Criteria | Weight | Score |
|----------|--------|-------|
| Product Quality | 20% | [ ] |
| Functional Testing | 20% | [ ] |
| Functionality | 10% | [ ] |
| Reliability | 10% | [ ] |
| Usability | 10% | [ ] |
| Efficiency | 10% | [ ] |
| Maintainability | 10% | [ ] |
| Portability | 10% | [ ] |
| **Total** | **100%** | **[ ]** |

---

## NEW SECTION: IV. Conclusion

**Add this entire section after Results and Discussion:**

### IV. CONCLUSION

This study successfully designed and developed a solar-powered automated fish feeding machine integrated with a real-time dissolved oxygen monitoring system. The prototype achieved its main objectives: automated scheduled feeding, real-time monitoring of dissolved oxygen, feed level, and battery status, SMS-based alerts for critical conditions, and sustainable solar-powered operation. The integration of a mobile application provided farmers with convenient remote monitoring and control through Bluetooth, while the ESP32-CAM offered live visual oversight of the pond.

Testing demonstrated that the system reliably automates feeding and reduces the need for constant manual supervision, addressing the labor-intensive nature of traditional aquaculture. The combination of automated feeding and water quality monitoring helps prevent both overfeeding and hypoxia-related fish losses, contributing to improved efficiency and sustainability in small to medium-scale fishpond operations.

For future work, the system can be enhanced with dissolved oxygen sensor calibration for improved measurement accuracy, long-range communication through WiFi or cellular networks for remote operation beyond Bluetooth range, self-stabilization or mobility for autonomous positioning, and support for monitoring multiple ponds from a single application.

---

## FIGURE TO REDRAW: Figure 3 (Block Diagram)

The current block diagram image still shows outdated labels. Redraw it with the following corrections:

| Current Label (WRONG) | Correct Label |
|----------------------|---------------|
| OLED Display | TFT LCD Display |
| Stepper Motor Driver | L298N Motor Driver |
| Load Cell | Ultrasonic Sensor |

**Also add these missing components to the diagram:**
- ESP32 (BLE bridge between Arduino and mobile app)
- ESP32-CAM (WiFi video stream)
- Servo Motor (hopper gate)
- Mobile Application (connected via BLE)

**Suggested data flow for the new diagram:**
```
Sensors (DO, Ultrasonic, Voltage) → Arduino Mega → ESP32 (BLE) → Mobile App
                                          ↓
                          Servo + DC Motor (L298N), SIM800L, TFT LCD, Buzzer
ESP32-CAM → WiFi → Mobile App (live video)
Solar Panel → MPPT → Battery → Buck Converter → 5V components
```

---

## CHECKLIST — Apply All Corrections

- [ ] Fix limitations paragraph (Correction 1)
- [ ] Replace "pellet" wording in 3 places (Correction 2)
- [ ] Expand Software Development section
- [ ] Add System Architecture subsection (II-D)
- [ ] Add Results and Discussion section (III) + fill in your data
- [ ] Add Conclusion section (IV)
- [ ] Redraw Figure 3 block diagram
- [ ] Fill in beta testing scores in Table 1
