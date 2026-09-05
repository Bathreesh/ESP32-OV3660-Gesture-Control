Below is a complete **GitHub README.md section for your Hand Gesture Control project**. You can copy this directly into your GitHub repository.

---

# ESP32-CAM Hand Gesture Controlled LED System

## 📌 Project Overview

This project is a real-time hand gesture recognition and wireless LED control system using an ESP32-CAM with an OV3660 camera module, Python, OpenCV, and MediaPipe.

The ESP32-CAM captures live video and transmits JPEG frames to a laptop through a TCP socket connection. The Python application receives the video stream and processes each frame using computer vision and hand landmark detection.

MediaPipe Hands detects the human hand and identifies finger positions. Based on the detected hand gesture, the Python program sends a control command back to the ESP32 through another TCP connection.

The ESP32 receives the command and controls its built-in white LED accordingly.

---

# 🎯 Objective

The main objective of this project is to develop a low-cost real-time hand gesture control system using an ESP32-CAM and computer vision.

The system allows a user to control an LED without physically touching any switch.

---

# 🖐️ Hand Gesture Controls

| Hand Gesture   | Fingers Detected | Command | LED Action          |
| -------------- | ---------------: | ------- | ------------------- |
| 🖐️ Open Palm  |      4/5 fingers | ON      | LED continuously ON |
| ✊ Closed Fist  |        0 fingers | OFF     | LED OFF             |
| ☝️ One Finger  |         1 finger | SLOW    | Slow blinking       |
| ✌️ Two Fingers |        2 fingers | FAST    | Fast blinking       |

---

# 🏗️ System Architecture

```text
                 ┌─────────────────────┐
                 │     Human Hand      │
                 │      Gesture        │
                 └──────────┬──────────┘
                            │
                            ▼
                 ┌─────────────────────┐
                 │   OV3660 Camera     │
                 │     ESP32-CAM       │
                 └──────────┬──────────┘
                            │
                      JPEG Video Frames
                            │
                      TCP Port 81
                            │
                            ▼
                 ┌─────────────────────┐
                 │   Python Program    │
                 │                     │
                 │ OpenCV + MediaPipe  │
                 └──────────┬──────────┘
                            │
                    Gesture Detection
                            │
                            ▼
                 ┌─────────────────────┐
                 │ Command Generation  │
                 │ ON / OFF / SLOW /   │
                 │ FAST                │
                 └──────────┬──────────┘
                            │
                      TCP Port 82
                            │
                            ▼
                 ┌─────────────────────┐
                 │      ESP32-CAM      │
                 │                     │
                 │ Built-in White LED  │
                 └─────────────────────┘
```

---

# ⚙️ Working Principle

The project works in two-way communication.

### Step 1: Video Capture

The OV3660 camera captures live video frames.

### Step 2: Video Transmission

The ESP32 compresses the captured frame into JPEG format.

Each JPEG frame is transmitted to the laptop using a TCP socket connection.

```text
ESP32 TCP Video Server
Port: 81
```

### Step 3: Video Processing

The Python application receives the JPEG data and reconstructs the image using OpenCV.

```text
TCP Data
    ↓
JPEG Frame
    ↓
OpenCV Decoding
    ↓
Live Video Display
```

### Step 4: Hand Detection

MediaPipe Hands processes each video frame.

It detects:

* Hand position
* Finger positions
* Hand landmarks
* Number of raised fingers

MediaPipe provides 21 landmark points for each detected hand.

### Step 5: Gesture Recognition

The Python program analyzes the finger positions and identifies the gesture.

For example:

```text
0 fingers → OFF
1 finger  → SLOW
2 fingers → FAST
4/5 fingers → ON
```

### Step 6: Command Transmission

After recognizing the gesture, Python sends a command to the ESP32.

A second TCP connection is used.

```text
ESP32 TCP Control Server
Port: 82
```

Commands sent:

```text
ON
OFF
SLOW
FAST
```

### Step 7: LED Control

The ESP32 receives the command and controls its built-in white LED.

---

# 🧠 Machine Learning Model Used

## MediaPipe Hands

This project uses the **MediaPipe Hands framework** for hand landmark detection.

MediaPipe Hands is an AI-based computer vision solution that detects hand landmarks in real time.

The model identifies 21 important points on the hand.

```text
          Finger Tips
              ●
              │
     ● ● ● ● ●
       Hand
     Landmarks
```

The detected landmarks are used to determine whether fingers are open or closed.

---

# 🔬 Technology Used

| Technology | Purpose                                |
| ---------- | -------------------------------------- |
| ESP32-CAM  | Main embedded controller               |
| OV3660     | Camera sensor                          |
| Python     | Main processing application            |
| OpenCV     | Image decoding and video display       |
| MediaPipe  | Hand landmark detection                |
| NumPy      | Image data processing                  |
| TCP Socket | Communication between ESP32 and laptop |
| Wi-Fi      | Wireless communication                 |
| GPIO       | LED control                            |

---

# 📦 Hardware Requirements

* ESP32-CAM development board
* OV3660 camera module
* Laptop or PC
* Wi-Fi network
* USB to TTL programmer (for uploading ESP32 code)
* Jumper wires
* Power supply

---

# 💻 Software Requirements

### ESP32 Side

* Arduino IDE
* ESP32 Board Package
* ESP32 Camera Library

### Python Side

* Python 3.12+
* OpenCV
* MediaPipe
* NumPy

Install required Python libraries:

```bash
py -m pip install opencv-python mediapipe numpy
```

---

# 🌐 Network Architecture

The ESP32-CAM and laptop must be connected to the same Wi-Fi network.

Example:

```text
Wi-Fi Router
     │
     ├──────── ESP32-CAM
     │
     └──────── Laptop
```

Example ESP32 IP:

```text
10.253.65.221
```

---

# 🔌 TCP Communication

This project uses two separate TCP servers.

## Video Server

```text
Port: 81
Purpose: Live video transmission
```

Data flow:

```text
ESP32 Camera
      ↓
JPEG Encoding
      ↓
TCP Socket Port 81
      ↓
Python OpenCV
```

---

## Gesture Control Server

```text
Port: 82
Purpose: Receive LED control commands
```

Data flow:

```text
MediaPipe Gesture Detection
          ↓
Python
          ↓
TCP Command
          ↓
ESP32 Port 82
          ↓
LED Control
```

---

# 📡 Communication Protocol

## Video Frame Protocol

Each video frame is transmitted as:

```text
┌───────────────────────┐
│ Frame Size (4 Bytes)  │
├───────────────────────┤
│                       │
│                       │
│      JPEG DATA        │
│                       │
│                       │
└───────────────────────┘
```

The Python program first receives the frame size.

Then it receives the complete JPEG image.

Example:

```text
4 Bytes Frame Size
        ↓
Receive JPEG Data
        ↓
OpenCV Decode
        ↓
Display Frame
```

---

# 🎮 Gesture Control Commands

| Command | Description   |
| ------- | ------------- |
| `ON`    | Turn LED ON   |
| `OFF`   | Turn LED OFF  |
| `SLOW`  | Slow blinking |
| `FAST`  | Fast blinking |

---

# 📂 Project Structure

```text
ESP32-CAM-Hand-Gesture-Control/
│
├── ESP32_Code/
│   └── esp32_ov3660_gesture_control.ino
│
├── Python_Code/
│   └── hand_gesture_control.py
│
├── README.md
│
└── requirements.txt
```

---

# 🐍 Python Processing Flow

```text
Receive TCP Frame
        ↓
Decode JPEG using OpenCV
        ↓
Convert BGR to RGB
        ↓
MediaPipe Hands Processing
        ↓
Detect Hand Landmarks
        ↓
Count Raised Fingers
        ↓
Identify Gesture
        ↓
Generate Command
        ↓
Send Command to ESP32
```

---

# 🔧 ESP32 Processing Flow

```text
Start ESP32
      ↓
Initialize PSRAM
      ↓
Initialize OV3660 Camera
      ↓
Connect Wi-Fi
      ↓
Start TCP Video Server
Port 81
      ↓
Start TCP Control Server
Port 82
      ↓
Wait for Python Connection
      ↓
Capture Camera Frame
      ↓
Send JPEG Frame
      ↓
Receive Gesture Command
      ↓
Control Built-in LED
```

---

# 📊 Camera Configuration

The OV3660 camera configuration used in this project:

| Parameter             | Value     |
| --------------------- | --------- |
| Resolution            | 640 × 480 |
| Format                | JPEG      |
| JPEG Quality          | 12        |
| XCLK                  | 10 MHz    |
| Frame Buffers         | 2         |
| Frame Buffer Location | PSRAM     |
| Sensor ID             | 0x3660    |

---

# ⚡ Why TCP Was Used Instead of HTTP Streaming

Initially, HTTP MJPEG streaming was tested.

However, the system experienced problems such as:

* Incomplete frame transmission
* Connection timeout
* JPEG send failures
* `FB-OVF` camera buffer overflow
* Client disconnection
* Stream instability

Example errors:

```text
cam_hal: FB-OVF
```

```text
JPEG data send failed
```

```text
Stream send timeout
```

Therefore, a custom TCP communication protocol was implemented.

TCP provided better control over:

* Frame size
* Data transmission
* Packet reception
* Error handling
* Frame reconstruction

The final TCP implementation successfully transmitted complete JPEG frames.

---

# 🧪 Development and Testing Process

The project was developed step by step.

### Test 1: ESP32 Wi-Fi Connection

The ESP32 successfully connected to Wi-Fi.

Example:

```text
Wi-Fi connected
ESP32 IP: 10.253.65.221
```

---

### Test 2: Camera Detection

The OV3660 sensor was successfully detected.

```text
Sensor ID: 0x3660
OV3660 initialized successfully
```

---

### Test 3: Camera Frame Rate Test

The camera was tested independently.

Results:

```text
Resolution: 640 × 480
Average FPS: Approximately 13 FPS
JPEG Size: Approximately 10–12 KB
```

This confirmed that the camera hardware was working correctly.

---

### Test 4: TCP Text Communication

A simple TCP connection was tested first.

ESP32 sent:

```text
HELLO_FROM_ESP32
```

Python successfully received multiple messages.

This confirmed stable TCP communication.

---

### Test 5: Large TCP Data Transfer

A 12,000-byte data packet was transmitted.

Python output:

```text
Expected data: 12000 bytes
Received: 12000 bytes

TEST PASSED!
```

This confirmed that large image-sized data could be transmitted successfully.

---

### Test 6: JPEG Video Transmission

The ESP32 transmitted JPEG frames through TCP.

Each frame contained:

```text
4 Byte Frame Size
+
JPEG Image Data
```

Python successfully reconstructed the frames.

---

### Test 7: Hand Detection

MediaPipe Hands was integrated with the live video.

The system successfully detected hand landmarks.

---

### Test 8: Gesture Recognition

Different finger configurations were tested.

```text
Open Palm → ON
Fist → OFF
One Finger → SLOW
Two Fingers → FAST
```

---

### Test 9: ESP32 LED Control

Commands were sent from Python to the ESP32.

The built-in LED responded according to the detected gesture.

---

# 📈 Performance

Approximate project performance:

| Parameter      | Result          |
| -------------- | --------------- |
| Resolution     | 640 × 480       |
| Camera FPS     | ~10–13 FPS      |
| Communication  | TCP             |
| Video Port     | 81              |
| Control Port   | 82              |
| Camera Sensor  | OV3660          |
| Processing     | Real-time       |
| Hand Detection | MediaPipe Hands |

Actual FPS may vary depending on:

* Wi-Fi signal strength
* Laptop performance
* CPU usage
* Camera lighting
* Distance from router

---

# ⚠️ Limitations

Current limitations include:

* Performance depends on Wi-Fi signal strength.
* Hand detection requires sufficient lighting.
* Fast hand movement may reduce gesture accuracy.
* ESP32-CAM has limited processing power.
* Video processing is performed on the laptop, not directly on ESP32.
* Only predefined gestures are supported.

---

# 🚀 Future Improvements

Possible future upgrades include:

* Control multiple LEDs.
* Control home appliances using relays.
* Add more hand gestures.
* Add gesture smoothing.
* Develop a mobile application.
* Use ESP32-S3 for edge AI processing.
* Add object detection.
* Add voice control.
* Implement gesture-based smart home automation.
* Record video to an SD card.
* Add cloud monitoring.

---

# 🏠 Applications

This project can be used in:

* Smart homes
* Touchless switches
* Home automation
* Assistive technology
* IoT systems
* Industrial control
* Gesture-controlled devices
* Educational computer vision projects

---

# 🎓 Learning Outcomes

Through this project, the following concepts were learned:

* ESP32-CAM programming
* OV3660 camera configuration
* TCP socket programming
* Wireless video transmission
* Python programming
* OpenCV image processing
* MediaPipe hand detection
* Hand landmark recognition
* ESP32 GPIO control
* Client-server architecture
* IoT communication

---

# 👨‍💻 Author

**Bathreesh M**

Project developed as an ESP32-CAM based Computer Vision and Gesture Control system.

---

# ⭐ Conclusion

This project successfully demonstrates a real-time touchless LED control system using hand gestures.

The ESP32-CAM with the OV3660 camera captures live video and transmits it wirelessly to a laptop through a TCP connection. Python processes the video using OpenCV and MediaPipe Hands.

The hand gesture is recognized based on finger positions and converted into control commands. These commands are sent back to the ESP32 through a separate TCP connection. The ESP32 then controls its built-in LED according to the detected gesture.

The project demonstrates the integration of:

```text
Embedded Systems
        +
Computer Vision
        +
Machine Learning / AI
        +
Wireless Communication
        +
IoT
```

to create a practical touchless control system.

---

## Recommended GitHub Repository Name

I suggest:

```text
ESP32-CAM-Hand-Gesture-Control
```

Or a more professional name:

```text
ESP32-OV3660-Gesture-Control-System
```
