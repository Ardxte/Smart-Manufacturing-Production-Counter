# Smart Manufacturing Production Counter

An embedded production monitoring system developed using the **STM32 Nucleo-F411RE** for a smart manufacturing application.

The system simulates a manufacturing production line by tracking accepted and rejected products, displaying production progress, and providing real-time production status through an LCD, 7-segment displays, and LEDs.

## Features

- Start and stop production
- Track accepted and rejected products
- Display production count using 7-segment displays
- Display production information and status on an LCD
- Calculate production efficiency
- Indicate active production, target reached, and fault status using LEDs
- Reset production data
- Button-based operator controls

## Hardware & Technologies

- **STM32 Nucleo-F411RE**
- **C/C++**
- **ARM Mbed**
- LCD Display
- 7-Segment Displays
- Push Buttons
- LEDs
- GPIO-based control

## Project Overview

The project demonstrates how a microcontroller can be used to monitor and control a simplified manufacturing process. Operator inputs are used to simulate product completion and rejection, while the system processes and displays production data in real time.

The project was developed as part of a **Microprocessor & Computer Architecture** project focused on applying embedded systems concepts to a smart manufacturing scenario.

## External Libraries

This project uses the following external libraries:

- [mbed-os Library](https://github.com/ARMmbed/mbed-os) — used for LCD display control.
- [Text LCD Library](https://github.com/PelionIoT/TextLCD/) — used for 7-segment display control.

## Project Information

**Members:** Ardiansyah, Wildan Owais Al Bukhari
**Semester:** May 2026

## Project Demo

▶️ [Project Demo](https://youtu.be/wEvm1UvAUZM)

The video demonstrates the production counter in operation, including:
- Starting and stopping production
- Counting accepted products
- Registering rejected products
- Displaying the production count
- Production efficiency calculation
- Target completion and status indicators
