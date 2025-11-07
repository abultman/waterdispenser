// WebSocket connection
let ws;
let currentState = 'idle';
let volumeUnit = 'ml'; // Default unit
let presetValues = [100, 250, 500, 1000]; // Default presets in ml

// Load volume unit preference from API
async function loadVolumeUnit() {
    try {
        const response = await fetch('/api/volumeunit');
        const data = await response.json();
        volumeUnit = data.unit || 'ml';
    } catch (error) {
        console.error('Failed to load volume unit:', error);
        volumeUnit = 'ml';
    }
}

// Save volume unit preference to API
async function saveVolumeUnit(unit) {
    volumeUnit = unit;
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

// Format volume based on user preference
function formatVolume(ml) {
    if (volumeUnit === 'l') {
        return (ml / 1000).toFixed(3) + ' L';
    }
    return ml.toFixed(1) + ' ml';
}

// Convert display value to ml for API
function convertToMl(value, unit) {
    if (unit === 'l') {
        return value * 1000;
    }
    return value;
}

// Set volume unit (called from config page)
function setVolumeUnit(unit) {
    saveVolumeUnit(unit);
    updateUnitDisplay();
    console.log('Volume unit changed to:', unit);
}

// Update UI elements based on volume unit
function updateUnitDisplay() {
    const unitHint = document.getElementById('unitHint');
    if (unitHint) {
        if (volumeUnit === 'l') {
            unitHint.textContent = 'Enter amount in liters';
        } else {
            unitHint.textContent = 'Enter amount in milliliters';
        }
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
    // Update volume unit if provided
    if (data.volumeUnit) {
        volumeUnit = data.volumeUnit;
        updateUnitDisplay();
    }

    // Update preset values if provided
    if (data.presets && data.presets.length === 4) {
        presetValues = data.presets;
        updatePresetButtons();
    }

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
        if (radio.value === volumeUnit) {
            radio.checked = true;
        }
    });
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
        // Always send in ml to the API
        const amountInMl = convertToMl(amount, volumeUnit);
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
