# ESP32 Water Dispenser System

A complete water dispensing system for ESP32 with touch interface, flow control, and WiFi connectivity.

## Hardware

- **ESP32**: CYD ESP32-8048S043
- **Display**: 800x480 ILI9485
- **Touch Controller**: GT911
- **Valve**: Normally closed, single pin control
- **Flow Sensor**: Pulse-based flow counter

## Features

### Touch Interface
- **Beautiful LVGL-based UI**: Multiple screens with intuitive navigation
- **Preset Buttons**: Quick access to common amounts (100ml, 250ml, 500ml, 1000ml)
- **Custom Amount**: Numeric keypad for entering any amount
- **Pause/Resume**: Pause dispensing mid-operation and resume when ready
- **Real-time Progress**: Visual progress bar and amount display during dispensing
- **Configuration Screen**: Easy WiFi setup and calibration access

### Web Interface (NEW!)
- **Remote Control**: Full control from any device on your network
- **Real-time Updates**: WebSocket-based live status updates
- **Responsive Design**: Works on phones, tablets, and desktops
- **All Features Available**: Preset buttons, custom amounts, pause/resume, settings
- **REST API**: Complete API for integration with home automation systems
- **No App Required**: Access via any web browser at `http://[ESP32-IP]`

## Pin Configuration

All pins are defined in `src/config.h` and can be easily changed:

### Display Pins
```cpp
#define TFT_MISO        -1
#define TFT_MOSI        11
#define TFT_SCLK        12
#define TFT_CS          10
#define TFT_DC          13
#define TFT_RST         14
#define TFT_BL          9    // Backlight
```

### Touch Controller Pins
```cpp
#define TOUCH_SDA       19
#define TOUCH_SCL       20
#define TOUCH_INT       21
#define TOUCH_RST       22
```

### Hardware Control Pins
```cpp
#define VALVE_PIN       23   // Valve control (HIGH = open)
#define FLOW_SENSOR_PIN 24   // Flow counter pulse input
```

## Installation

1. **Open in VS Code with PlatformIO**
   - Open this folder in VS Code
   - PlatformIO should automatically detect the project

2. **Configure Pins** (if needed)
   - Edit `src/config.h` to match your hardware connections
   - Update flow sensor calibration factor if known

3. **Upload Filesystem (Web Interface)**
   - **IMPORTANT**: You must upload the web interface files before the firmware
   - In PlatformIO, click on "Platform" in the left sidebar
   - Click "Upload Filesystem Image" (or use the command `pio run --target uploadfs`)
   - This uploads the HTML, CSS, and JavaScript files to the ESP32's LittleFS
   - Wait for upload to complete (takes ~30 seconds)

4. **Build and Upload Firmware**
   - Connect your ESP32 via USB
   - Click "Upload" in PlatformIO (or use `pio run --target upload`)
   - Monitor serial output for debugging

**Note**: If you make changes to the web interface (files in `data/`), you need to run "Upload Filesystem Image" again. Firmware uploads do not affect the filesystem.

## Configuration

### Flow Sensor Calibration

The flow sensor needs to be calibrated for accurate dispensing:

1. Go to **Settings** → **Calibrate Flow Sensor**
2. Enter a known volume (e.g., 1000 ml)
3. Press **Start** and dispense the known volume into a measuring container
4. Press **Save** when complete
5. The system calculates and saves the calibration factor

Default calibration: `450 pulses/liter` (adjust in `config.h`)

### WiFi Setup

1. Go to **Settings**
2. Enter your WiFi SSID and password
3. Press **Connect**
4. Credentials are saved for future use

### Adjustable Settings in config.h

```cpp
// Flow sensor calibration
#define DEFAULT_PULSES_PER_LITER  450.0

// Dispensing timeout
#define FLOW_TIMEOUT    5000  // ms

// Overshoot compensation
#define OVERSHOOT_COMPENSATION  5.0  // ml

// Preset amounts
#define PRESET_1_ML     100
#define PRESET_2_ML     250
#define PRESET_3_ML     500
#define PRESET_4_ML     1000

// Display brightness
#define DEFAULT_BRIGHTNESS  200  // 0-255
```

## Usage

### Touch Screen Interface

#### Main Screen
- **Preset Buttons**: Tap to dispense preset amounts instantly
- **Custom Amount**: Tap to enter a specific amount via keypad
- **Settings**: Access WiFi and calibration settings

#### Keypad Screen
- Enter desired amount in milliliters
- Press **Start** to begin dispensing
- Press **Cancel** to return to main screen

#### Dispensing Screen
- Shows current dispensed amount and target
- Progress bar with percentage
- **PAUSE** button to pause dispensing
- When paused: **RESUME** or **STOP** buttons
- Automatically returns to main screen when complete

#### Settings Screen
- **WiFi Configuration**: Connect to your network
- **Calibrate Flow Sensor**: Run calibration routine
- Status display shows connection info

### Web Interface

#### Accessing the Web Interface
1. Connect the ESP32 to your WiFi network (via touchscreen or web)
2. Note the IP address displayed on the touch screen or serial monitor
3. Open any web browser and navigate to `http://[ESP32-IP]`
4. The interface works on phones, tablets, and desktop computers

#### Web Interface Features
- **Quick Dispense**: Tap preset buttons (100ml, 250ml, 500ml, 1000ml)
- **Custom Amount**: Enter any amount and press Start
- **Real-time Monitoring**: Watch dispensing progress live with WebSocket updates
- **Pause/Resume**: Full control during dispensing
- **WiFi Configuration**: Change network settings remotely
- **Calibration**: Adjust flow sensor calibration factor
- **Status Display**: See WiFi info, IP address, and current state

#### Web Interface Architecture
The web interface uses **LittleFS** (ESP32's filesystem) to serve static files:
- HTML, CSS, and JavaScript files are stored in the `data/` directory
- Files are uploaded separately to the ESP32 using "Upload Filesystem Image"
- This saves RAM by not storing web pages in program memory
- Easy to modify - just edit files in `data/` and re-upload filesystem
- The web server serves files directly from LittleFS at runtime

#### REST API Endpoints

For home automation integration:

```bash
# Get system status
GET /api/status

# Start dispensing (amount in ml)
POST /api/start
  amount=500

# Pause dispensing
POST /api/pause

# Resume dispensing
POST /api/resume

# Stop dispensing
POST /api/stop

# Configure WiFi
POST /api/wifi
  ssid=YourSSID
  password=YourPassword

# Get calibration
GET /api/calibration

# Set calibration
POST /api/calibration
  pulsesPerLiter=450.0
```

#### WebSocket Connection
- Connect to `ws://[ESP32-IP]/ws` for real-time status updates
- Receives JSON status every 500ms when clients are connected
- Automatic reconnection on disconnect

## Troubleshooting

### Touch Not Working
- Check I2C connections (SDA, SCL)
- Verify GT911 address (0x5D or 0x14)
- Check power supply

### Display Issues
- Verify SPI connections
- Check backlight pin
- Try different rotation settings

### Flow Sensor Not Detecting
- Verify interrupt pin connection
- Check sensor power supply
- Run calibration routine
- Ensure water is flowing

### Valve Not Operating
- Check valve pin connection
- Verify valve power supply
- Test valve manually with `hardwareControl.openValve()`

### WiFi Connection Failed
- Verify SSID and password
- Check 2.4GHz WiFi (ESP32 doesn't support 5GHz)
- Move closer to router during setup

### Web Interface Not Loading
- **Did you upload the filesystem?** Run "Upload Filesystem Image" in PlatformIO first!
- Verify ESP32 is connected to WiFi (check touch screen or serial monitor)
- Ensure you're on the same network as the ESP32
- Try accessing via IP address directly (shown on touch screen)
- Check serial monitor for "LittleFS mounted successfully" message
- Check firewall settings
- Clear browser cache and try again

### WebSocket Not Connecting
- Some corporate/public WiFi networks block WebSocket connections
- Try a different network
- Check browser console for error messages
- REST API will still work even if WebSocket fails

## Serial Monitor

Connect via serial at **115200 baud** to see debug output:
- System initialization
- WiFi connection status
- Dispensing progress
- Flow sensor pulse counts
- Calibration data

## Code Structure

```
src/
├── config.h              # All pin definitions and settings
├── main.cpp              # Main application and setup
├── HardwareControl.h/cpp # Valve and flow sensor control
├── UIManager.h/cpp       # LVGL UI implementation
├── WebServer.h/cpp       # Web server and REST API
├── GT911.h/cpp           # Touch controller driver
└── lv_conf.h             # LVGL configuration

include/
└── User_Setup.h          # TFT_eSPI display configuration

data/                     # Web interface files (uploaded to LittleFS)
├── index.html            # Main web interface
├── style.css             # Styling
└── app.js                # JavaScript logic and WebSocket
```

## Dependencies

All dependencies are automatically installed by PlatformIO:
- TFT_eSPI (display driver)
- LVGL (UI library)
- ESPAsyncWebServer (web server)
- AsyncTCP (async networking)
- ArduinoJson (JSON serialization)
- WiFi (built-in)
- Preferences (settings storage)
- Wire (I2C communication)

## License

Open source - feel free to modify and use for your projects!

## Support

For issues or questions:
1. Check serial monitor for error messages
2. Verify all pin connections match config.h
3. Test individual components (display, touch, valve, sensor)
4. Run calibration routine

## Future Enhancements

Possible additions:
- Water level monitoring
- Usage statistics and logging
- Web interface for remote control
- Multiple user profiles
- Scheduled dispensing
- Temperature monitoring
