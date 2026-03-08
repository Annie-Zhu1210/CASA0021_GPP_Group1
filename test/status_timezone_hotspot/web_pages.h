// HTML pages served by the captive portal hotspot.

#ifndef WEB_PAGES_H
#define WEB_PAGES_H

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Device Setup</title>
    <style>
        body { 
            font-family: Arial, sans-serif; 
            max-width: 400px; 
            margin: 50px auto; 
            padding: 20px;
            background-color: #f0f0f0;
        }
        .container {
            background-color: white;
            padding: 30px;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        h2 { 
            color: #333; 
            text-align: center;
        }
        input { 
            width: 100%; 
            padding: 12px; 
            margin: 8px 0; 
            box-sizing: border-box;
            border: 1px solid #ddd;
            border-radius: 4px;
        }
        button { 
            width: 100%; 
            background-color: #4CAF50; 
            color: white; 
            padding: 14px; 
            border: none; 
            border-radius: 4px;
            cursor: pointer;
            font-size: 16px;
            margin-top: 10px;
        }
        button:hover { 
            background-color: #45a049; 
        }
        .info {
            background-color: #e7f3fe;
            padding: 10px;
            border-left: 4px solid #2196F3;
            margin-bottom: 20px;
            font-size: 14px;
        }
        .list {
            margin: 8px 0 12px 0;
            max-height: 180px;
            overflow-y: auto;
            border: 1px solid #ddd;
            border-radius: 6px;
            background: #fff;
        }
        .item {
            width: 100%;
            text-align: left;
            margin: 0;
            border: 0;
            border-bottom: 1px solid #eee;
            border-radius: 0;
            background: #fff;
            color: #222;
            padding: 10px 12px;
            font-size: 14px;
            cursor: pointer;
        }
        .itemRow {
            display: flex;
            align-items: center;
            justify-content: space-between;
            gap: 10px;
        }
        .ssidText {
            overflow: hidden;
            text-overflow: ellipsis;
            white-space: nowrap;
            max-width: 230px;
        }
        .sigWrap {
            display: flex;
            align-items: center;
            gap: 6px;
            color: #666;
            font-size: 12px;
            flex-shrink: 0;
        }
        .wifiBars {
            display: inline-flex;
            align-items: flex-end;
            gap: 2px;
            height: 12px;
        }
        .wifiBars i {
            display: inline-block;
            width: 3px;
            background: #c7c7c7;
            border-radius: 1px;
        }
        .wifiBars i:nth-child(1) { height: 4px; }
        .wifiBars i:nth-child(2) { height: 8px; }
        .wifiBars i:nth-child(3) { height: 12px; }
        .wifiBars i.on { background: #2e7d32; }
        .item:last-child { border-bottom: 0; }
        .item:hover { background: #f6fbff; }
        .muted { color: #666; font-size: 12px; }
        .selectedBadge {
            margin-top: 8px;
            padding: 8px 10px;
            background: #eef7ff;
            border: 1px solid #cfe7ff;
            border-radius: 6px;
            color: #1f4f7a;
            font-size: 12px;
            line-height: 1.4;
        }
        .row {
            display: flex;
            align-items: center;
            justify-content: space-between;
            margin-top: 6px;
        }
        .row button {
            width: auto;
            margin-top: 0;
            padding: 10px 12px;
            font-size: 14px;
        }
        details {
            margin-top: 10px;
            border: 1px solid #ddd;
            border-radius: 6px;
            padding: 8px 10px;
            background: #fafafa;
        }
        summary {
            cursor: pointer;
            color: #333;
            font-weight: 600;
        }
        .pairingBlock {
            margin-top: 14px;
            padding-top: 10px;
            border-top: 1px dashed #ddd;
        }
        #passwordBlock { display: none; }
    </style>
</head>
<body>
    <div class="container">
        <h2>Long Distance Device</h2>
        <div class="info">
            Enter your home Wi-Fi credentials and a pairing code to connect with your partner's device.<br>
            <strong>Note:</strong> This device supports <strong>2.4GHz Wi-Fi only</strong> (5GHz is not supported).
        </div>
        <form action="/save" method="POST">
            <label>Choose Wi-Fi Network:</label>
            <div class="row">
                <div class="muted" id="selectedText">No network selected</div>
                <button type="button" id="scanBtn">Scan Wi-Fi</button>
            </div>
            <div class="selectedBadge" id="selectedBadge">Currently selected: none</div>
            <div class="list" id="wifiList"></div>

            <input id="ssidInput" type="hidden" name="ssid">

            <div id="passwordBlock">
                <label>Wi-Fi Password:</label>
                <input id="passwordInput" type="password" name="password" required placeholder="Your Wi-Fi Password">
            </div>

            <details>
                <summary>Manual input</summary>
                <label>Wi-Fi Network Name:</label>
                <input id="manualSsid" type="text" placeholder="Your Wi-Fi SSID">
                <label>Wi-Fi Password:</label>
                <input id="manualPassword" type="password" placeholder="Your Wi-Fi Password">
                <button type="button" id="useManualBtn">Use this network</button>
            </details>
            
            <div class="pairingBlock">
                <label>Pairing Code:</label>
                <input type="text" name="pairing" required placeholder="Share this with your partner">
            </div>
            
            <button type="submit">Save & Connect</button>
        </form>
    </div>
    <script>
        const form = document.querySelector('form');
        const scanBtn = document.getElementById('scanBtn');
        const wifiList = document.getElementById('wifiList');
        const selectedText = document.getElementById('selectedText');
        const selectedBadge = document.getElementById('selectedBadge');
        const ssidInput = document.getElementById('ssidInput');
        const passwordBlock = document.getElementById('passwordBlock');
        const passwordInput = document.getElementById('passwordInput');
        const manualSsid = document.getElementById('manualSsid');
        const manualPassword = document.getElementById('manualPassword');
        const useManualBtn = document.getElementById('useManualBtn');
        let selectedSource = '';

        function escapeHtml(str) {
            return String(str)
                .replaceAll('&', '&amp;')
                .replaceAll('<', '&lt;')
                .replaceAll('>', '&gt;')
                .replaceAll('"', '&quot;')
                .replaceAll("'", '&#39;');
        }

        function signalLevel(rssi) {
            if (rssi >= -60) return { bars: 3, label: 'Strong' };
            if (rssi >= -75) return { bars: 2, label: 'Medium' };
            return { bars: 1, label: 'Weak' };
        }

        function chooseSsid(ssid, open) {
            ssidInput.value = ssid;
            selectedText.textContent = 'Selected: ' + ssid;
            passwordBlock.style.display = open ? 'none' : 'block';
            passwordInput.required = !open;
            if (open) passwordInput.value = '';
            const src = selectedSource ? selectedSource : 'List';
            selectedBadge.textContent = 'Currently selected: ' + ssid + ' (' + src + ')';
        }

        function renderList(items) {
            wifiList.innerHTML = '';
            if (!items || !items.length) {
                wifiList.innerHTML = '<div class="muted" style="padding:10px;">No networks found. You can still use Manual input.</div>';
                return;
            }
            items.sort((a, b) => b.rssi - a.rssi);
            items.forEach(n => {
                if (!n.ssid) return;
                const sig = signalLevel(n.rssi);
                const b1 = sig.bars >= 1 ? 'on' : '';
                const b2 = sig.bars >= 2 ? 'on' : '';
                const b3 = sig.bars >= 3 ? 'on' : '';
                const sec = n.open ? 'Open' : 'Secured';
                const btn = document.createElement('button');
                btn.type = 'button';
                btn.className = 'item';
                btn.innerHTML =
                    '<div class="itemRow">' +
                        '<span class="ssidText">' + escapeHtml(n.ssid) + ' <span class="muted">[' + sec + ']</span></span>' +
                        '<span class="sigWrap">' +
                            '<span class="wifiBars">' +
                                '<i class="' + b1 + '"></i>' +
                                '<i class="' + b2 + '"></i>' +
                                '<i class="' + b3 + '"></i>' +
                            '</span>' +
                            '<span>' + sig.label + '</span>' +
                        '</span>' +
                    '</div>';
                btn.onclick = () => {
                    selectedSource = 'List';
                    chooseSsid(n.ssid, !!n.open);
                };
                wifiList.appendChild(btn);
            });
        }

        async function scanWifi() {
            scanBtn.disabled = true;
            scanBtn.textContent = 'Scanning...';
            try {
                const res = await fetch('/scan');
                const data = await res.json();
                renderList(data);
            } catch (e) {
                wifiList.innerHTML = '<div class="muted" style="padding:10px;">Scan failed. Try again or use Manual input.</div>';
            } finally {
                scanBtn.disabled = false;
                scanBtn.textContent = 'Scan Wi-Fi';
            }
        }

        useManualBtn.addEventListener('click', () => {
            const s = (manualSsid.value || '').trim();
            if (!s) return;
            selectedSource = 'Manual';
            chooseSsid(s, false);
            passwordInput.value = manualPassword.value || '';
            selectedText.textContent = 'Selected (manual): ' + s;
        });

        form.addEventListener('submit', (e) => {
            if (!ssidInput.value) {
                const ms = (manualSsid.value || '').trim();
                if (ms) {
                    selectedSource = 'Manual';
                    chooseSsid(ms, false);
                    passwordInput.value = manualPassword.value || '';
                    selectedText.textContent = 'Selected (manual): ' + ms;
                }
            }
            if (!ssidInput.value) {
                e.preventDefault();
                alert('Please select a Wi-Fi network or use Manual input first.');
            }
        });

        scanBtn.addEventListener('click', scanWifi);
        scanWifi();
    </script>
</body>
</html>
)rawliteral";

const char success_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Success</title>
    <style>
        body { 
            font-family: Arial, sans-serif; 
            text-align: center; 
            padding: 50px;
            background-color: #f0f0f0;
        }
        .success {
            background-color: white;
            padding: 40px;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            max-width: 400px;
            margin: 0 auto;
        }
        h2 { color: #4CAF50; }
    </style>
</head>
<body>
    <div class="success">
        <h2> Configuration Saved!</h2>
        <p>Your device is now connecting to Wi-Fi.</p>
        <p>The setup network will close in a few seconds.</p>
    </div>
</body>
</html>
)rawliteral";

#endif
