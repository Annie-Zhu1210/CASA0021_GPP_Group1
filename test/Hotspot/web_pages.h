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
    </style>
</head>
<body>
    <div class="container">
        <h2>Long Distance Device</h2>
        <div class="info">
            Enter your home Wi-Fi credentials and a pairing code to connect with your partner's device.
        </div>
        <form action="/save" method="POST">
            <label>Wi-Fi Network Name:</label>
            <input type="text" name="ssid" required placeholder="Your Wi-Fi SSID">
            
            <label>Wi-Fi Password:</label>
            <input type="password" name="password" required placeholder="Your Wi-Fi Password">
            
            <label>Pairing Code:</label>
            <input type="text" name="pairing" required placeholder="Share this with your partner">
            
            <button type="submit">Save & Connect</button>
        </form>
    </div>
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