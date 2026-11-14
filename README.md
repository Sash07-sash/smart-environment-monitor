# Smart Environment Monitoring System

IoT telemetry and automation system built with ESP32, Firebase Cloud Firestore, and a responsive web dashboard.

## Overview
- **Microcontroller**: ESP32 Dev Module
- **Sensors**: DHT11 (Temperature & Humidity), LDR (Light Intensity)
- **Actuation**: 5V Relay / LED Bulb
- **Cloud Backend**: Google Cloud Firebase (Authentication & Cloud Firestore)
- **Frontend**: Glassmorphic responsive web dashboard with real-time updates and CSV export
- **Live Deployment**: https://smart-environment-monito-3c43f.web.app

## Project Structure
- `public/`: Web dashboard source files (HTML, CSS, JavaScript) hosted on Firebase.
- `firmware/`: ESP32 Arduino sketches for Tasks 1 through 5.
- `firestore.rules`: Cloud Firestore security rules.
- `firebase.json`: Firebase Hosting configuration.

## Features
1. Real-time sensor monitoring (Temperature, Humidity, Light).
2. Dual-mode appliance control (Manual toggle vs Automatic threshold).
3. Historical logging in Cloud Firestore.
4. Tabular CSV data export.
5. Firebase Email/Password authentication.

## Author
Satheesh B - ProtoSem IoT Batch 1.18
