# Build Scripts

## compress_web_files.py

This script automatically compresses web files (HTML, CSS, JS) using gzip before uploading to the ESP32's LittleFS filesystem.

### How it works

1. **Automatic compression**: Runs before building the filesystem image
2. **Pre-compressed files**: Creates `.gz` versions of web files in the `data/` directory
3. **Server optimization**: ESPAsyncWebServer automatically serves `.gz` files when available
4. **Bandwidth savings**: Typically reduces file sizes by 60-80%

### Files compressed

- `*.html` - HTML pages
- `*.css` - Stylesheets
- `*.js` - JavaScript files
- `*.json` - JSON data
- `*.svg` - SVG images

### Manual compression

To manually compress files without building:

```bash
cd scripts
python3 compress_web_files_manual.py
```

### How the server uses compressed files

When a browser requests `/app.js`, ESPAsyncWebServer will:
1. Check if `/app.js.gz` exists
2. If yes, serve the `.gz` file with `Content-Encoding: gzip` header
3. If no, serve the original `/app.js`

The browser automatically decompresses the gzipped content.

### Benefits

- **Faster page loads**: Less data to transfer
- **Lower bandwidth**: Important for ESP32's limited network speed
- **No runtime overhead**: Files are pre-compressed, not compressed on-the-fly
- **Better UX**: Web interface loads much faster

### Typical compression ratios

- HTML: 70-80% reduction
- CSS: 75-85% reduction
- JavaScript: 60-70% reduction
- JSON: 80-90% reduction

### Build process integration

The script is automatically run during PlatformIO build via `platformio.ini`:

```ini
extra_scripts =
    pre:scripts/compress_web_files.py
```
