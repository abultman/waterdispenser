// ============================================================
// Volume Unit System (similar to C++ VolumeUnit pattern)
// ============================================================

class VolumeUnit {
    toDisplay(milliliters) {
        throw new Error('Must implement toDisplay()');
    }

    toMilliliters(displayValue) {
        throw new Error('Must implement toMilliliters()');
    }

    format(milliliters) {
        throw new Error('Must implement format()');
    }

    getSuffix() {
        throw new Error('Must implement getSuffix()');
    }

    getType() {
        throw new Error('Must implement getType()');
    }
}

class MillilitersUnit extends VolumeUnit {
    toDisplay(milliliters) {
        return milliliters;
    }

    toMilliliters(displayValue) {
        return displayValue;
    }

    format(milliliters) {
        return milliliters.toFixed(1);
    }

    getSuffix() {
        return 'ml';
    }

    getType() {
        return 'ml';
    }
}

class LitersUnit extends VolumeUnit {
    toDisplay(milliliters) {
        return milliliters / 1000.0;
    }

    toMilliliters(displayValue) {
        return displayValue * 1000.0;
    }

    format(milliliters) {
        return (milliliters / 1000.0).toFixed(3);
    }

    getSuffix() {
        return 'L';
    }

    getType() {
        return 'l';
    }
}

// Factory function to get unit instance
function getVolumeUnit(type) {
    if (type === 'l') {
        return new LitersUnit();
    }
    return new MillilitersUnit();
}

// ============================================================
// Application State
// ============================================================

let ws;
let currentState = 'idle';
let volumeUnitType = 'ml'; // Current unit type ('ml' or 'l')
let currentVolumeUnit = getVolumeUnit('ml'); // Current unit instance
let presetValues = [100, 250, 500, 1000]; // Default presets in ml

// Load volume unit preference from API
async function loadVolumeUnit() {
    try {
        const response = await fetch('/api/volumeunit');
        const data = await response.json();
        volumeUnitType = data.unit || 'ml';
        currentVolumeUnit = getVolumeUnit(volumeUnitType);
    } catch (error) {
        console.error('Failed to load volume unit:', error);
        volumeUnitType = 'ml';
        currentVolumeUnit = getVolumeUnit('ml');
    }
}

// Save volume unit preference to API
async function saveVolumeUnit(unit) {
    volumeUnitType = unit;
    currentVolumeUnit = getVolumeUnit(unit);
    try {
        await apiCall('/api/volumeunit', 'POST', { unit: unit });
    } catch (error) {
        console.error('Failed to save volume unit:', error);
    }
}

// Load preset values from API
async function loadPresets() {
    try {
        const response = await fetch('/api/presets');
        const data = await response.json();
        presetValues = [data.preset1, data.preset2, data.preset3, data.preset4];
    } catch (error) {
        console.error('Failed to load presets:', error);
        presetValues = [100, 250, 500, 1000];
    }
}

// Format volume based on current unit (with suffix)
function formatVolume(ml) {
    return currentVolumeUnit.format(ml) + ' ' + currentVolumeUnit.getSuffix();
}

// Set volume unit (called from config page)
async function setVolumeUnit(unit) {
    await saveVolumeUnit(unit);
    updateUnitDisplay();
    // Reload presets to update display with new unit
    await loadPresets();
    updatePresetButtons();
    // Update preset input fields if on config page
    updatePresetInputs();
    console.log('Volume unit changed to:', unit);
}

// Update UI elements based on volume unit
function updateUnitDisplay() {
    const unitHint = document.getElementById('unitHint');
    if (unitHint) {
        const suffix = currentVolumeUnit.getSuffix();
        const fullName = volumeUnitType === 'l' ? 'liters' : 'milliliters';
        unitHint.textContent = `Enter amount in ${fullName}`;
    }
}

// Connect to WebSocket
function connectWebSocket() {
    ws = new WebSocket(`ws://${window.location.hostname}/ws`);

    ws.onopen = () => {
        console.log('WebSocket connected');
    };

    ws.onmessage = (event) => {
        const data = JSON.parse(event.data);
        updateUI(data);
    };

    ws.onclose = () => {
        console.log('WebSocket disconnected, reconnecting...');
        setTimeout(connectWebSocket, 3000);
    };

    ws.onerror = (error) => {
        console.error('WebSocket error:', error);
    };
}

// Update preset buttons with current values
function updatePresetButtons() {
    const presetGrid = document.querySelector('.preset-grid');
    if (presetGrid && presetValues.length === 4) {
        presetGrid.innerHTML = '';
        presetValues.forEach((ml) => {
            const button = document.createElement('button');
            button.className = 'btn btn-primary';
            button.textContent = formatVolume(ml);
            button.onclick = () => startDispensing(ml);
            presetGrid.appendChild(button);
        });
    }
}

// Update UI with data from server
function updateUI(data) {
    // Note: volumeUnit and presets are loaded separately via API calls
    // They are not included in the WebSocket status updates to improve performance

    // Update WiFi status
    const wifiStatusEl = document.getElementById('wifiStatus');
    if (wifiStatusEl) {
        if (data.wifi.connected) {
            wifiStatusEl.textContent = `WiFi: ${data.wifi.ssid} (${data.wifi.ip})`;
        } else {
            wifiStatusEl.textContent = 'WiFi: Not connected';
        }
    }

    // Update state badge (main page)
    const stateBadge = document.getElementById('stateStatus');
    if (stateBadge) {
        currentState = data.dispensing.state;
        stateBadge.textContent = data.dispensing.state.toUpperCase();
        stateBadge.className = 'status-badge';

        if (data.dispensing.state === 'idle' || data.dispensing.state === 'completed') {
            stateBadge.classList.add('status-idle');
            const mainSection = document.getElementById('mainSection');
            const dispensingSection = document.getElementById('dispensingSection');
            if (mainSection) mainSection.classList.remove('hidden');
            if (dispensingSection) dispensingSection.classList.add('hidden');
        } else if (data.dispensing.state === 'dispensing') {
            stateBadge.classList.add('status-dispensing');
            const mainSection = document.getElementById('mainSection');
            const dispensingSection = document.getElementById('dispensingSection');
            if (mainSection) mainSection.classList.add('hidden');
            if (dispensingSection) dispensingSection.classList.remove('hidden');
            const controlsDispensing = document.getElementById('controlsDispensing');
            const controlsPaused = document.getElementById('controlsPaused');
            if (controlsDispensing) controlsDispensing.classList.remove('hidden');
            if (controlsPaused) controlsPaused.classList.add('hidden');
        } else if (data.dispensing.state === 'paused') {
            stateBadge.classList.add('status-paused');
            const mainSection = document.getElementById('mainSection');
            const dispensingSection = document.getElementById('dispensingSection');
            if (mainSection) mainSection.classList.add('hidden');
            if (dispensingSection) dispensingSection.classList.remove('hidden');
            const controlsDispensing = document.getElementById('controlsDispensing');
            const controlsPaused = document.getElementById('controlsPaused');
            if (controlsDispensing) controlsDispensing.classList.add('hidden');
            if (controlsPaused) controlsPaused.classList.remove('hidden');
        } else if (data.dispensing.state.includes('error')) {
            stateBadge.classList.add('status-error');
        }
    }

    // Update dispensing info (main page)
    const dispensedEl = document.getElementById('dispensedAmount');
    const targetEl = document.getElementById('targetAmount');
    const remainingEl = document.getElementById('remainingAmount');
    if (dispensedEl) dispensedEl.textContent = formatVolume(data.dispensing.dispensed);
    if (targetEl) targetEl.textContent = formatVolume(data.dispensing.target);
    if (remainingEl) remainingEl.textContent = formatVolume(data.dispensing.remaining);

    const progressBar = document.getElementById('progressBar');
    if (progressBar) {
        const progress = data.dispensing.progress;
        progressBar.style.width = progress + '%';
        progressBar.textContent = progress + '%';
    }

    // Update calibration display
    const calibrationEl = document.getElementById('currentCalibration');
    if (calibrationEl) {
        calibrationEl.textContent = data.calibration.pulsesPerLiter.toFixed(2) + ' pulses/L';
    }

    // Update config page WiFi info
    updateConfigPageInfo(data);
}

// Update config page specific elements
function updateConfigPageInfo(data) {
    const wifiConnectedStatus = document.getElementById('wifiConnectedStatus');
    const currentSsid = document.getElementById('currentSsid');
    const currentIp = document.getElementById('currentIp');
    const signalStrength = document.getElementById('signalStrength');

    if (wifiConnectedStatus) {
        if (data.wifi.connected) {
            wifiConnectedStatus.textContent = '✓ Connected';
            wifiConnectedStatus.style.color = '#27AE60';
        } else {
            wifiConnectedStatus.textContent = '✗ Not Connected';
            wifiConnectedStatus.style.color = '#E74C3C';
        }
    }

    if (currentSsid) currentSsid.textContent = data.wifi.ssid || '-';
    if (currentIp) currentIp.textContent = data.wifi.ip || '-';
    if (signalStrength && data.wifi.rssi) {
        const rssi = data.wifi.rssi;
        let quality = 'Excellent';
        if (rssi < -80) quality = 'Poor';
        else if (rssi < -70) quality = 'Fair';
        else if (rssi < -60) quality = 'Good';
        signalStrength.textContent = `${quality} (${rssi} dBm)`;
    }
}

// Initialize config page
async function initConfigPage() {
    await loadVolumeUnit();
    await loadPresets();

    // Set the correct radio button
    const radios = document.querySelectorAll('input[name="volumeUnit"]');
    radios.forEach(radio => {
        if (radio.value === volumeUnitType) {
            radio.checked = true;
        }
    });

    // Populate preset input fields with current values
    updatePresetInputs();
}

// Update preset input fields based on current unit
function updatePresetInputs() {
    const preset1Input = document.getElementById('preset1');
    const preset2Input = document.getElementById('preset2');
    const preset3Input = document.getElementById('preset3');
    const preset4Input = document.getElementById('preset4');

    if (preset1Input && presetValues.length === 4) {
        // Convert from ml to display unit using VolumeUnit
        preset1Input.value = currentVolumeUnit.format(presetValues[0]);
        preset2Input.value = currentVolumeUnit.format(presetValues[1]);
        preset3Input.value = currentVolumeUnit.format(presetValues[2]);
        preset4Input.value = currentVolumeUnit.format(presetValues[3]);
    }
}

// Save presets to API
async function savePresets() {
    const preset1Input = document.getElementById('preset1');
    const preset2Input = document.getElementById('preset2');
    const preset3Input = document.getElementById('preset3');
    const preset4Input = document.getElementById('preset4');

    if (!preset1Input || !preset2Input || !preset3Input || !preset4Input) {
        return;
    }

    // Get values from inputs (in display unit)
    const p1 = parseFloat(preset1Input.value);
    const p2 = parseFloat(preset2Input.value);
    const p3 = parseFloat(preset3Input.value);
    const p4 = parseFloat(preset4Input.value);

    // Validate
    if (p1 <= 0 || p2 <= 0 || p3 <= 0 || p4 <= 0) {
        alert('All preset values must be greater than 0');
        return;
    }

    // Convert to milliliters for storage using VolumeUnit
    const preset1_ml = currentVolumeUnit.toMilliliters(p1);
    const preset2_ml = currentVolumeUnit.toMilliliters(p2);
    const preset3_ml = currentVolumeUnit.toMilliliters(p3);
    const preset4_ml = currentVolumeUnit.toMilliliters(p4);

    // Save via API
    const result = await apiCall('/api/presets', 'POST', {
        preset1: preset1_ml,
        preset2: preset2_ml,
        preset3: preset3_ml,
        preset4: preset4_ml
    });

    if (result.success) {
        alert('Presets saved successfully!');
        // Reload presets and update buttons
        await loadPresets();
        updatePresetButtons();
    } else {
        alert('Failed to save presets: ' + (result.error || 'Unknown error'));
    }
}

// API call helper
async function apiCall(endpoint, method = 'GET', body = null) {
    const options = {
        method: method,
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' }
    };

    if (body) {
        options.body = new URLSearchParams(body);
    }

    try {
        const response = await fetch(endpoint, options);
        return await response.json();
    } catch (error) {
        console.error('API call failed:', error);
        return { success: false, error: error.message };
    }
}

// Control functions
async function startDispensing(amount) {
    await apiCall('/api/start', 'POST', { amount: amount });
}

async function startCustomAmount() {
    const amountInput = document.getElementById('customAmount');
    if (!amountInput) return;

    const amount = parseFloat(amountInput.value);
    if (amount && amount > 0) {
        // Convert to ml using VolumeUnit before sending to API
        const amountInMl = currentVolumeUnit.toMilliliters(amount);
        await startDispensing(amountInMl);
        amountInput.value = '';
    }
}

async function pauseDispensing() {
    await apiCall('/api/pause', 'POST');
}

async function resumeDispensing() {
    await apiCall('/api/resume', 'POST');
}

async function stopDispensing() {
    await apiCall('/api/stop', 'POST');
}

async function configureWiFi() {
    const ssid = document.getElementById('wifiSsid').value;
    const password = document.getElementById('wifiPassword').value;

    if (ssid) {
        const result = await apiCall('/api/wifi', 'POST', {
            ssid: ssid,
            password: password
        });

        if (result.success) {
            alert('WiFi configuration saved. Device is connecting...');
        }
    }
}

async function saveCalibration() {
    const factor = document.getElementById('calibrationFactor').value;

    if (factor && factor > 0) {
        const result = await apiCall('/api/calibration', 'POST', {
            pulsesPerLiter: factor
        });

        if (result.success) {
            alert('Calibration saved!');
            document.getElementById('calibrationFactor').value = '';
        }
    }
}

// Initialize on page load
async function initializePage() {
    await loadVolumeUnit();
    await loadPresets();
    updateUnitDisplay();
    updatePresetButtons();
    connectWebSocket();
}

initializePage();

// Allow Enter key to submit custom amount (if element exists)
const customAmountInput = document.getElementById('customAmount');
if (customAmountInput) {
    customAmountInput.addEventListener('keypress', (e) => {
        if (e.key === 'Enter') startCustomAmount();
    });
}
