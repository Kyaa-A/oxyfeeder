OxyFeeder: System Research & Core Concepts
Version: 1.0
Source: "OxyFeeder: IOT-based Stand-Alone Solar-Powered System for Automated Fish Feeding and Dissolved Oxygen Monitoring System for Fishponds." (Gemina, Racho, Sarcauga)

1. Abstract
OxyFeeder is a stand-alone, solar-powered IoT system designed to modernize aquaculture practices for small to medium fishponds. It addresses critical challenges in traditional fish farming by automating daily feeding schedules and providing continuous, real-time monitoring of dissolved oxygen (DO) levels. The system's primary goal is to reduce manual labor, prevent massive fish kills due to hypoxia, optimize feed usage, and promote sustainable, off-grid farming through renewable energy.

2. Problem Domain
Traditional aquaculture is hindered by several key challenges:
- Labor-Intensive Processes: Daily manual feeding is time-consuming and prone to human error.
- Resource Inefficiency: Manual feeding often leads to overfeeding (wasted feed, poor water quality) or underfeeding (stunted fish growth).
- Hypoxia Risk: Sudden drops in dissolved oxygen are a primary cause of mass fish kills. Manual spot-checks are often inadequate for prevention.
- Off-Grid Constraints: Access to reliable grid electricity for monitoring and automation is often unavailable or cost-prohibitive in rural farming areas.

3. Core System Objectives
The project is designed to achieve the following:
- Automate Actions: Automatically dispense feed based on a user-defined schedule and continuously track real-time dissolved oxygen (DO) levels.
- Provide Rich Data: Integrate key features for efficient pond management, including feed inventory tracking, an app-based alert system for critical notifications (low oxygen, low feed), and camera monitoring.
- Ensure Sustainability: Incorporate a robust solar power system to ensure sustainable and reliable off-grid operation.
- Validate Performance: The system must be reliable and functional in real-world or simulated aquaculture conditions.

4. System Logic Flow (Input-Process-Output Model)
The system operates on a clear, three-stage logic cycle:

INPUTS:
- User-Defined Settings: Feeding schedule, critical warning values (e.g., low battery alert level), target phone number for alerts.
- Environmental Factors: Available solar power from the panel.
- Physical Inputs: Fish feed added to the hopper by the user.
- Sensor Data: Continuous real-time data from the DO sensor, load cell, and voltage sensor.

PROCESS (Microcontroller Operations):
- Constant Monitoring: The Arduino Mega continuously reads sensor data.
- Power Management: The MPPT charge controller optimizes solar energy use and battery charging.
- Decision Logic: The firmware compares sensor values against user-defined thresholds.
- Scheduled Actuation: The RTC module triggers the Arduino to execute the motor control logic at scheduled feeding times.
- Alert Preparation: The ESP32 formats and prepares notifications when critical thresholds are breached.

OUTPUTS:
- Automated Fish Feeding: The mechanical dispenser activates at scheduled times.
- Real-Time Data Display: The on-device OLED screen shows current feed level, battery status, time, and DO levels.
- App/SMS Notifications: Alerts for "Low Feed," "Low Battery," or "Critical DO Level" are sent to the user's phone.
- Local Alerts: Onboard LEDs and a buzzer provide immediate status indication at the device location.

5. Key Hardware Components
- Main Controller: Arduino Mega 2560 (manages sensors, motors, RTC, OLED).
- Communications Controller: ESP32 (manages Wi-Fi, Bluetooth, and GSM module).
- Power System: 12V Solar Panel, MPPT Charge Controller, 12V Battery.

Core Sensors:
- Gravity Analog Dissolved Oxygen Sensor: Measures O2 in water.
- Load Cell + HX711 Amplifier: Measures weight of remaining feed.
- DS3231 RTC Module: Keeps precise time for scheduling.
- Voltage Sensor: Monitors battery charge level.

Actuators:
- 12V DC Gear Motor with Spinner Plate
- L298N Motor Driver

Connectivity:
- SIM800L GSM Module for SMS alerts

On-Device UI:
- 0.96" OLED Display
