# OPC UA ESP32 Server

An OPC UA server implementation for ESP32 microcontrollers, enabling industrial IoT communication protocols on embedded devices.

## About

This project implements an OPC UA server on ESP32 hardware, allowing the device to communicate using the OPC UA protocol - a standard for industrial automation and IoT applications. The implementation is based on [opcua-esp32](https://github.com/cmbahadir/opcua-esp32) library.
## Features

- OPC UA server running on ESP32
- Industrial-standard communication protocol
- Support for data nodes and variables
- WiFi connectivity
- Configurable server endpoints

## Hardware Requirements

- ESP32 development board
- USB cable for programming and power
- WiFi network for connectivity

## Installation

### Prerequisites

- [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/) development framework
- Git

### Setup

1. Clone the repository:
```bash
git clone https://github.com/Mikor01/DesignLab_OPC_SERVER.git
cd DesignLab_OPC_SERVER/esp32
```

2. Configure your WiFi credentials and server settings, using provided template:
```bash
cp sdkconfig.default sdkconfig
idf.py menuconfig
-> Connection settings
```

3. Build the project:
```bash
idf.py build
```

4. Flash to your ESP32:
```bash
idf.py -p (PORT) flash monitor
```

Replace `(PORT)` with your ESP32's serial port (e.g., `/dev/ttyUSB0` on Linux or `COM3` on Windows).

## Usage

Once flashed and running, the ESP32 will:
1. Connect to the configured WiFi network
2. Start the OPC UA server
3. Expose data nodes accessible via OPC UA clients

You can connect to the server using any OPC UA client (such as UAExpert or Prosys OPC UA Browser) using the server's IP address and configured endpoint.

### To use the included Python clients, you need to set up a virtual environment:
1. Setup Environment:
```bash
# Create venv
python -m venv venv

# Activate (Windows)
.\venv\Scripts\activate
# Activate (Linux/macOS)
source venv/bin/activate

# Install requirements
pip install opcua
```
2. Run Client
- GUI Version: ```bash python GUI_OPC_client.py```

- Terminal Version: ```bash python OPC_client.py```

## Acknowledgments

This project is built upon:
- [open62541](https://github.com/open62541/open62541) - An open source OPC UA implementation
- [opcua-esp32](https://github.com/cmbahadir/opcua-esp32) - ESP32 port by cmbahadir


## Project Progress
30.10.25 - initial commits and testing  
06.11.25 - Added basic UART functionalities  
13.11.25 - Modified UART to be compliant with pico commands  
20.11.25 - Added status command to Pico, refined UART code on ESP
27.11.25 - Final touches in regards to main functionality
<4,11,18>.12.25 - Extensive stability testing and bug fixes
<8,15,22>.01.26 - Python OPC client TUI / GUI developement and minor bug fixes
29.01.26 - Final presentation
