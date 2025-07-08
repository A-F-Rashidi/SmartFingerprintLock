# SmartFingerprintLock

This is a **smart lock system** built using an **ESP32**, a **fingerprint sensor**, and an **OLED display**. Upon successful fingerprint recognition, the system sends an **email notification** with the exact date and time of access. It also includes a user interface via a button for enrolling or deleting fingerprints.

## 🚀 Features

- 🔎 Fingerprint recognition using Adafruit sensor
- 📬 Email alert when the lock is accessed (SMTP over Gmail)
- 📺 Real-time feedback via OLED screen
- 📡 WiFi and NTP time sync
- 🔘 Button-based fingerprint management (enroll/delete)
- 🔊 Buzzer feedback for access result

---

## 🛠️ Hardware Requirements

- ESP32 board (e.g., DOIT DevKit V1)
- R307 or similar fingerprint sensor (UART)
- OLED 128x64 (I2C, SSD1306)
- Buzzer
- Push button
- Jumper wires and breadboard

### 📷 Example Wiring

| Component         | ESP32 Pin     |
|------------------|---------------|
| Fingerprint RX   | GPIO 16       |
| Fingerprint TX   | GPIO 17       |
| OLED SDA         | GPIO 21 (default) |
| OLED SCL         | GPIO 22 (default) |
| Button           | GPIO 27       |
| Buzzer           | GPIO 15       |

---

## 📚 Required Libraries

Install the following libraries via the Arduino Library Manager:

- `Adafruit_Fingerprint`
- `Adafruit_SSD1306`
- `Adafruit_GFX`
- `ESP_Mail_Client`

---

## 🔐 Email Setup

1. Use a **Gmail account** and enable **App Passwords** from your Google account security settings.
2. Create an app password (16-character).
3. In the code, use:

```cpp
#define AUTHOR_EMAIL "your_email@gmail.com"
#define AUTHOR_PASSWORD "your_app_password"

## ✍️ Author

**Amirhosien Farshbaf Rashidi*  
ESP32 Security Projects - 2025
