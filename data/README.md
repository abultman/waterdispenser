# Web Interface Files

This directory contains the web interface files that are uploaded to the ESP32's LittleFS filesystem.

## Files

- **index.html** - Main dispensing page (clean, focused on water dispensing)
- **config.html** - Configuration page (WiFi, calibration, preferences)
- **style.css** - All styling and responsive design
- **app.js** - JavaScript logic, WebSocket connection, API calls, and multi-page support

## How to Upload

### Using PlatformIO

1. **Via UI:**
   - Open the PlatformIO sidebar in VS Code
   - Click on "Platform"
   - Click "Upload Filesystem Image"
   - Wait for upload to complete (~30 seconds)

2. **Via Command Line:**
   ```bash
   pio run --target uploadfs
   ```

### Important Notes

- **Upload filesystem BEFORE uploading firmware** on first setup
- The filesystem upload is separate from the firmware upload
- Modifying files in this directory requires re-uploading the filesystem
- Firmware uploads do NOT affect filesystem contents
- The ESP32 must be connected via USB for filesystem upload

## Modifying the Web Interface

1. Edit the HTML, CSS, or JavaScript files in this directory
2. Run "Upload Filesystem Image" in PlatformIO
3. Refresh your browser (you may need to hard refresh: Ctrl+Shift+R or Cmd+Shift+R)

## File Size Considerations

- Total filesystem size is limited (typically 1-4MB depending on partition scheme)
- Keep files reasonably sized for faster uploads and better performance
- Consider minifying CSS/JS for production if size becomes an issue

## Testing Locally

You can test the web interface locally by:
1. Opening `index.html` in a browser
2. The WebSocket and API calls won't work, but you can test the UI layout
3. Use browser developer tools to inspect and debug

## Architecture

The web interface is a single-page application (SPA) that:
- Uses vanilla JavaScript (no frameworks required)
- Connects via WebSocket for real-time updates
- Falls back to REST API for commands
- Works offline once loaded (as long as ESP32 is reachable on network)
