// bring in
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <time.h>


// make an wifi ap
const char* ap_ssid = "ESP32-Setup";
const char* ap_password = "12345678";


// geteway / mask
IPAddress local_IP(192, 168, 4, 1); // no 0 ir 1 section for defult, will be conflect with upstream
IPAddress gateway(192, 168, 4, 1); // can not  be 192.168.0.1 
IPAddress subnet(255, 255, 255, 0);


// STA ONLINE // set up in dashboard
String staSSID = "DemoWiFi";
String staPassword = "DemoPassword";


// NTP
// time update
// can use dashbaord to set urls later
const char* ntpserver1 = "pool.ntp.org";
const char* ntpserver2 = "time.nist.gov";


// LA timezone
const char* tzInfo = "PST8PDT,M3.2.0/2,M11.1.0/2";


// weather update
// around LA, set at dashboard later
const char* weatherUrl =
  "https://api.open-meteo.com/v1/forecast?"
  "latitude=34.05&longitude=-118.24&current=temperature_2m,weather_code"
  "&timezone=America%2FLos_Angeles";



// web server listening port
WebServer server(80);


// backend cahce
String timeCache = "syncing...";
String weatherCache = "Syncing...";
float tempC = NAN;
int weatherCode = -1;
unsigned long lastWeatherMs = 0;


// scheduler / state
unsigned long lastSTATryMs = 0;
unsigned long lastWeatherTryMs = 0;
unsigned long lastTimeKickMs = 0;
bool staWasConnected = false;
bool timeSynced = false;

// =========================
// Demo Game State (Since I dont have other group's keys)
// =========================
// I just make up some, IDK LOL
struct PlayerState {
  String playerId;
  int level;
  int xp;
  int coins;
  int hp;
  int atk;
  int def;
  int passCount;
  int adventureCount;
  int battleCount;
  int riskScore;
  unsigned long lastPassMs;
};

// Default change later if needed
PlayerState player = {
  "ESP32-PLAYER-001",
  1,
  0,
  20,
  100,
  10,
  5,
  0,
  0,
  0,
  0,
  0
};




// =========================
// Anti-cheat Log Framwork
// =========================
// debug the log if needed
struct CheatLog {
  String eventType;
  String severity;
  String reason;
  bool accepted;
  unsigned long ms;
};

// Storage const, dont explode aut clean
const int CHEAT_LOG_SIZE = 10;
CheatLog cheatLogs[CHEAT_LOG_SIZE];
int cheatLogHead = 0;
int cheatLogCount = 0;



// small safe json text escape
String jsonEscape(const String& s) {
  String out;
  out.reserve(s.length() + 8);
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '\\') out += "\\\\";
    else if (c == '"') out += "\\\"";
    else if (c == '\n') out += "\\n";
    else if (c == '\r') out += "\\r";
    else out += c;
  }
  return out;
}


// use json num to storage weather, without using lib
// small json lib
String extractJsonObject(const String& src, const String& key) {
  String keyPattern = "\"" + key + "\"";
  int keyPos = src.indexOf(keyPattern);
  if (keyPos < 0) return "";


  int colonPos = src.indexOf(':', keyPos + keyPattern.length());
  if (colonPos < 0) return "";


  int start = colonPos + 1;
  while (start < (int)src.length() &&
         (src[start] == ' ' || src[start] == '\n' || src[start] == '\r' || src[start] == '\t')) {
    start++;
  }


  if (start >= (int)src.length() || src[start] != '{') return "";


  int depth = 0;
  for (int i = start; i < (int)src.length(); i++) {
    char c = src[i];
    if (c == '{') depth++;
    else if (c == '}') {
      depth--;
      if (depth == 0) {
        return src.substring(start, i + 1);
      }
    }
  }


  return "";
}


String extractJsonValue(const String& src, const String& key) {
  String pattern = "\"" + key + "\":";
  int start = src.indexOf(pattern);
  if (start < 0) return "";


  start += pattern.length();


  while (start < (int)src.length() &&
         (src[start] == ' ' || src[start] == '\n' || src[start] == '\r' || src[start] == '\t')) {
    start++;
  }


  if (start < (int)src.length() && src[start] == '"') {
    start++;
    int end = src.indexOf('"', start);
    if (end < 0) return "";
    return src.substring(start, end);
  }


  int end = start;
  while (end < (int)src.length()) {
    char c = src[end];
    if (c == ',' || c == '}' || c == ']' || c == '\n' || c == '\r') {
      break;
    }
    end++;
  }


  String out = src.substring(start, end);
  out.trim();
  return out;
}



// storage to weather
String weatherCodeToText(int code) {
  switch (code) {
    case 0: return "Clear";
    case 1: return "Mainly Clear";
    case 2: return "Partly Clear";
    case 3: return "Overcast";
    case 45:
    case 48: return "Fog";
    case 51:
    case 53:
    case 55: return "Drizzle";
    case 61:
    case 63:
    case 65: return "Rain";
    case 71:
    case 73:
    case 75: return "Snow";
    case 80:
    case 81:
    case 82: return "Rain showers";
    case 95: return "Thunderstorm";
    default: return "Code " + String(code);
  }
}


// start STA connect, NON-BLOCKING + set up in dashboard
void connectSTAAsync() {
  if (staSSID.length() == 0) {
    Serial.println("STA SSID empty, skip connect");
    return;
  }


  if (WiFi.status() == WL_CONNECTED) return;


  if (millis() - lastSTATryMs < 10000UL) return;


  Serial.print("STA connecting to: ");
  Serial.println(staSSID);


  WiFi.begin(staSSID.c_str(), staPassword.c_str());
  lastSTATryMs = millis();
}



// get time, NON-BLOCKING style
void kickTimeSync() {
  if (WiFi.status() != WL_CONNECTED) return;


  unsigned long retryGap = timeSynced ? 3600000UL : 15000UL; // 1h or 15s
  if (millis() - lastTimeKickMs < retryGap) return;


  configTzTime(tzInfo, ntpserver1, ntpserver2);
  lastTimeKickMs = millis();
  Serial.println("NTP sync kicked");
}


// update cached local time quickly
void updateTimeCache() {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 50)) {
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    timeCache = String(buf);
    timeSynced = true;
  } else if (!timeSynced) {
    timeCache = "syncing...";
  }
}

// write a test func
bool testWeatherSocketRaw() {
  const char* host = "api.open-meteo.com"; // api
  const int port = 443; // https 


  IPAddress ip;
  Serial.println("#### WEATHER TEST ####");
  Serial.printf("Free heap before DNS: %u\n", ESP.getFreeHeap());


  // check dns
  if (!WiFi.hostByName(host, ip)) {
    Serial.println("DNS resolve failed");
    weatherCache = "DNS failed";
    return false;
  }

  // check ip
  Serial.print("Resolved IP: ");
  Serial.println(ip);
  Serial.printf("Free heap before TLS connect: %u\n", ESP.getFreeHeap());


  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(8000);


  if (!client.connect(host, port)) {
    Serial.println("TLS connect failed");
    weatherCache = "TLS connect failed";
    return false;
  }


  Serial.println("TLS connect OK");
  Serial.printf("Free heap after TLS connect: %u\n", ESP.getFreeHeap());


  String path =
    "/v1/forecast?latitude=34.05&longitude=-118.24"
    "&current=temperature_2m,weather_code"
    "&timezone=America%2FLos_Angeles";


  client.print(String("GET ") + path + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: ESP32\r\n" +
               "Accept: application/json\r\n" +
               "Connection: close\r\n\r\n");


  unsigned long start = millis();
  while (!client.available() && client.connected() && millis() - start < 8000) {
    delay(10);
  }


  if (!client.available()) {
    Serial.println("No response payload");
    weatherCache = "No response";
    client.stop();
    return false;
  }


  String response;
  while (client.available()) {
    response += client.readString();
  }
  client.stop();


  Serial.println("----- RAW RESPONSE BEGIN -----");
  Serial.println(response);
  Serial.println("----- RAW RESPONSE END -----");


  int bodyPos = response.indexOf("\r\n\r\n");
  if (bodyPos < 0) {
    weatherCache = "Bad HTTP response";
    return false;
  }


  String body = response.substring(bodyPos + 4);


String currentObj = extractJsonObject(body, "current");
if (currentObj == "") {
  weatherCache = "No current object";
  Serial.println("RAW parse failed: current object missing");
  return false;
}


Serial.println("==== RAW CURRENT OBJ ====");
Serial.println(currentObj);


String timeVal = extractJsonValue(currentObj, "time");
String tempVal = extractJsonValue(currentObj, "temperature_2m");
String codeVal = extractJsonValue(currentObj, "weather_code");


  if (timeVal == "" || tempVal == "" || codeVal == "") {
    weatherCache = "Parse Failed";
    return false;
  }


  tempC = tempVal.toFloat();
  weatherCode = codeVal.toInt();
  weatherCache = weatherCodeToText(weatherCode) + ", " + String(tempC, 1) + " C";
  lastWeatherMs = millis();


  return true;
}


// get weather
bool fetchWeatherNow() {
  if (WiFi.status() != WL_CONNECTED) {
    weatherCache = "Wifi Offline";
    return false;
  }


  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(8000);   // give more timeout time


  HTTPClient http;
  http.setConnectTimeout(8000);
  http.setTimeout(8000);
  http.setReuse(false);
  http.useHTTP10(true);


  Serial.println("[WEATHER] begin connect...");


  if (!http.begin(client, weatherUrl)) {
    weatherCache = "HTTP begin failed";
    Serial.println("[WEATHER] http.begin failed");
    return false;
  }


  http.addHeader("Accept", "application/json");
  http.addHeader("User-Agent", "ESP32");


  int code = http.GET();


  Serial.print("[WEATHER] GET code = ");
  Serial.println(code);


  if (code <= 0) {
    String err = http.errorToString(code);
    weatherCache = "GET fail: " + err;
    Serial.print("[WEATHER] error: ");
    Serial.println(err);
    http.end();
    return false;
  }


  Serial.print("[WEATHER] HTTP status = ");
  Serial.println(code);


  String payload = http.getString();
  http.end();


  Serial.println("[WEATHER] payload:");
  Serial.println(payload);


  String timeVal = extractJsonValue(payload, "time");
  String tempVal = extractJsonValue(payload, "temperature_2m");
  String codeVal = extractJsonValue(payload, "weather_code");


  if (timeVal == "" || tempVal == "" || codeVal == "") {
    weatherCache = "Parse Failed";
    Serial.println("[WEATHER] parse failed");
    return false;
  }


  tempC = tempVal.toFloat();
  weatherCode = codeVal.toInt();
  weatherCache = weatherCodeToText(weatherCode) + ", " + String(tempC, 1) + " C";
  lastWeatherMs = millis();


  Serial.print("[WEATHER] ok: ");
  Serial.println(weatherCache);


  return true;
}


// scheduler
void refreshDataIfNeeded() {
  bool staConnected = (WiFi.status() == WL_CONNECTED);


  if (staConnected && !staWasConnected) {
    Serial.print("STA Connected, IP: ");
    Serial.println(WiFi.localIP());
  }


  if (!staConnected && staWasConnected) {
    Serial.println("STA disconnected");
  }


  staWasConnected = staConnected;


  // keep STA alive
  if (!staConnected) {
    connectSTAAsync();
  }


  // keep time cache alive
  kickTimeSync();
  updateTimeCache();


  // weather retry: 15 sec before first success, then every 10 min
  if (WiFi.status() == WL_CONNECTED) {
    unsigned long gap = (lastWeatherMs == 0) ? 15000UL : 600000UL;
    if (millis() - lastWeatherTryMs >= gap) {
      lastWeatherTryMs = millis();
      fetchWeatherNow();   // still blocking, but now NOT inside HTTP handler
    }
  } else {
    weatherCache = "Wifi Offline";
  }
}


// home page a lot of HTML CSS JS :(
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
 <meta name="viewport" content="width=device-width, initial-scale=1">
 <title>ESP32 StreetPass Dashboard</title>
 <style>
   body {
     font-family: Arial, sans-serif;
     background:#f5f5f5;
     margin:0;
     padding:24px;
     color:#222;
   }


   .wrap {
     max-width: 760px;
     margin: 0 auto;
   }


   .card {
     background:#fff;
     border-radius:16px;
     padding:20px;
     margin-bottom:16px;
     box-shadow:0 4px 16px rgba(0,0,0,.08);
   }


   h1 {
     margin:0 0 8px 0;
     font-size:28px;
   }


   h2 {
     margin:0 0 12px 0;
     font-size:20px;
   }


   .label {
     color:#666;
     font-size:14px;
     margin-bottom:6px;
   }


   .value {
     font-size:22px;
     font-weight:700;
     white-space:pre-line;
   }


   .muted {
     color:#777;
     font-size:13px;
     white-space:pre-line;
     line-height:1.5;
   }


   .grid {
     display:grid;
     grid-template-columns:1fr 1fr;
     gap:12px;
   }


   .stat {
     background:#f7f7f7;
     border-radius:12px;
     padding:12px;
   }


   .stat .k {
     color:#666;
     font-size:12px;
   }


   .stat .v {
     font-weight:700;
     font-size:18px;
     margin-top:4px;
   }


   input {
     width:100%;
     box-sizing:border-box;
     padding:10px;
     border:1px solid #ddd;
     border-radius:10px;
     margin:6px 0 10px 0;
     font-size:14px;
   }


   button {
     border:0;
     padding:10px 14px;
     border-radius:999px;
     background:#222;
     color:#fff;
     cursor:pointer;
     margin:4px 4px 4px 0;
   }


   button:hover {
     opacity:.85;
   }


   .danger {
     background:#b91c1c;
   }


   .good {
     background:#15803d;
   }


   .blue {
     background:#1d4ed8;
   }


   .log {
     border-left:4px solid #ddd;
     padding:8px 10px;
     margin:8px 0;
     background:#fafafa;
     border-radius:8px;
     font-size:13px;
   }


   .ok {
     border-left-color:#15803d;
   }


   .bad {
     border-left-color:#b91c1c;
   }


   @media (max-width: 640px) {
     .grid {
       grid-template-columns:1fr;
     }
   }
 </style>
</head>
<body>
 <div class="wrap">
   <div class="card">
     <h1>ESP32 StreetPass Dashboard</h1>
     <div class="muted">
       AP + STA setup / HTTP API / Mock StreetPass data / Anti-cheat validation frame (DEMO ONLY!!!!)
     </div>
   </div>


   <div class="card">
     <h2>System Status</h2>
     <div class="grid">
       <div class="stat">
         <div class="k">Current Time</div>
         <div class="v" id="time">Loading...</div>
       </div>
       <div class="stat">
         <div class="k">Weather</div>
         <div class="v" id="weather">Loading...</div>
       </div>
     </div>
     <br>
     <div class="label">Network</div>
     <div class="muted" id="net">Loading...</div>
   </div>


   <div class="card">
     <h2>WiFi Setup</h2>
     <form method="POST" action="/api/wifi">
       <div class="label">WiFi SSID</div>
       <input name="ssid" placeholder="Enter WiFi SSID" required>
       <div class="label">WiFi Password</div>
       <input name="password" type="password" placeholder="Enter WiFi Password">
       <button class="blue" type="submit">Save WiFi</button>
     </form>
     <div class="muted">
       Device keeps AP mode alive while trying to connect as STA.
     </div>
   </div>


   <div class="card">
     <h2>Player / StreetPass State</h2>
     <div class="grid">
       <div class="stat"><div class="k">Player ID</div><div class="v" id="player_id">-</div></div>
       <div class="stat"><div class="k">Level</div><div class="v" id="level">-</div></div>
       <div class="stat"><div class="k">XP</div><div class="v" id="xp">-</div></div>
       <div class="stat"><div class="k">Coins</div><div class="v" id="coins">-</div></div>
       <div class="stat"><div class="k">HP</div><div class="v" id="hp">-</div></div>
       <div class="stat"><div class="k">ATK / DEF</div><div class="v" id="stats">-</div></div>
       <div class="stat"><div class="k">Pass Count</div><div class="v" id="pass_count">-</div></div>
       <div class="stat"><div class="k">Risk Score</div><div class="v" id="risk_score">-</div></div>
     </div>


     <br>
     <button class="good" onclick="postAction('/api/sim/pass')">Simulate StreetPass</button>
     <button class="good" onclick="postAction('/api/sim/adventure')">Simulate Adventure</button>
     <button class="good" onclick="postAction('/api/sim/battle')">Simulate Battle</button>
     <button class="danger" onclick="postAction('/api/sim/cheat')">Simulate Cheat</button>
     <button onclick="postAction('/api/sim/reset')">Reset</button>


     <div class="muted" id="action_msg"></div>
   </div>


   <div class="card">
     <h2>Anti-cheat Log</h2>
     <div class="muted">
       Events are validated before applying to player state. Suspicious events are blocked and logged.
     </div>
     <div id="logs"></div>
   </div>
 </div>


 <script>
   async function refreshSystem() {
     try {
       const r = await fetch('/api/status', { cache: 'no-store' });
       const data = await r.json();


       document.getElementById('time').innerText = data.time;
       document.getElementById('weather').innerText = data.weather;
       document.getElementById('net').innerText =
         "AP IP: " + data.ap_ip +
         "\\nSTA IP: " + data.sta_ip +
         "\\nClients: " + data.clients +
         "\\nSTA connected: " + data.sta_connected;
     } catch (e) {
       document.getElementById('net').innerText = "System API read failed: " + e;
     }
   }


   async function refreshGame() {
     try {
       const r = await fetch('/api/game', { cache: 'no-store' });
       const data = await r.json();


       document.getElementById('player_id').innerText = data.player_id;
       document.getElementById('level').innerText = data.level;
       document.getElementById('xp').innerText = data.xp;
       document.getElementById('coins').innerText = data.coins;
       document.getElementById('hp').innerText = data.hp;
       document.getElementById('stats').innerText = data.atk + " / " + data.def;
       document.getElementById('pass_count').innerText = data.pass_count;
       document.getElementById('risk_score').innerText = data.risk_score;
     } catch (e) {
       document.getElementById('action_msg').innerText = "Game API read failed: " + e;
     }
   }


   async function refreshAntiCheat() {
     try {
       const r = await fetch('/api/anticheat', { cache: 'no-store' });
       const data = await r.json();


       const box = document.getElementById('logs');
       box.innerHTML = "";


       if (!data.logs || data.logs.length === 0) {
         box.innerHTML = "<div class='muted'>No anti-cheat events yet.</div>";
         return;
       }


       data.logs.forEach(log => {
         const div = document.createElement('div');
         div.className = "log " + (log.accepted ? "ok" : "bad");
         div.innerText =
           "[" + (log.accepted ? "ACCEPTED" : "BLOCKED") + "] " +
           log.event_type + "\\n" +
           log.reason + "\\n" +
           "ms=" + log.ms;
         box.appendChild(div);
       });
     } catch (e) {
       document.getElementById('logs').innerText = "Anti-cheat API read failed: " + e;
     }
   }


   async function postAction(url) {
     try {
       const r = await fetch(url, { method: 'POST', cache: 'no-store' });
       const text = await r.text();
       document.getElementById('action_msg').innerText = text;
       refreshAll();
     } catch (e) {
       document.getElementById('action_msg').innerText = "Action failed: " + e;
     }
   }


   function refreshAll() {
     refreshSystem();
     refreshGame();
     refreshAntiCheat();
   }


   refreshAll();
   setInterval(refreshAll, 5000);
 </script>
</body>
</html>
)rawliteral";


  server.send(200, "text/html", html);
}



// API testing
void handleStatus() {
  // IMPORTANT:
  // DO NOT call refreshDataIfNeeded() here.
  // HTTP handler should only return cache, never do long network jobs.


  String json = "{";
  json += "\"time\":\"" + jsonEscape(timeCache) + "\",";
  json += "\"weather\":\"" + jsonEscape(weatherCache) + "\",";
  json += "\"ap_ip\":\"" + WiFi.softAPIP().toString() + "\",";
  json += "\"sta_ip\":\"" + WiFi.localIP().toString() + "\",";
  json += "\"clients\":" + String(WiFi.softAPgetStationNum()) + ",";
  json += "\"sta_connected\":" + String((WiFi.status() == WL_CONNECTED) ? "true" : "false");
  json += "}";


  server.send(200, "application/json", json);
}


// set wif ssid and password no search
void handleWifiConfig() {
  if (!server.hasArg("ssid")) {
    server.send(400, "text/plain", "Missing ssid");
    return;
  }


  String newSSID = server.arg("ssid");
  String newPassword = server.arg("password");


  newSSID.trim();


  if (newSSID.length() == 0) {
    server.send(400, "text/plain", "SSID cannot be empty");
    return;
  }


  staSSID = newSSID;
  staPassword = newPassword;


  Serial.print("[WIFI CONFIG] New SSID: ");
  Serial.println(staSSID);


  WiFi.disconnect(false, true);


  lastSTATryMs = 0;
  staWasConnected = false;
  timeSynced = false;
  lastWeatherMs = 0;
  weatherCache = "Wifi reconnecting...";
  timeCache = "syncing...";


  connectSTAAsync();


  server.sendHeader("Location", "/");
  server.send(303, "text/plain", "Redirecting...");
}



// 404
void handleNotFound() {
  server.send(404, "text/plain", "404 Not Found");
}




// =========================
// Anti cheat helper demo only if anything not right block
// =========================
// make up some level up speed example
int expectedLevelFromXP(int xp) {
  if (xp < 0) return 1;
  return 1 + (xp / 100);
}


int maxAtkForLevel(int level) {
  return 20 + level * 5;
}


int maxDefForLevel(int level) {
  return 10 + level * 3;
}


// count risk
void addRisk(int points) {
  player.riskScore += points;
  if (player.riskScore > 100) player.riskScore = 100;
}


// make up sone reason :)
void recordAntiCheat(const String& eventType, const String& severity, const String& reason, bool accepted) {
  cheatLogs[cheatLogHead].eventType = eventType;
  cheatLogs[cheatLogHead].severity = severity;
  cheatLogs[cheatLogHead].reason = reason;
  cheatLogs[cheatLogHead].accepted = accepted;
  cheatLogs[cheatLogHead].ms = millis();


  cheatLogHead = (cheatLogHead + 1) % CHEAT_LOG_SIZE;


  if (cheatLogCount < CHEAT_LOG_SIZE) {
    cheatLogCount++;
  }


  Serial.print("[ANTI-CHEAT] ");
  Serial.print(accepted ? "ACCEPTED " : "BLOCKED ");
  Serial.print(eventType);
  Serial.print(" - ");
  Serial.println(reason);
}


// =========================
// Anti cheat validator
// =========================
//  event can not edit stat directly, must pass through validator
bool validateGameEvent(
  const String& eventType,
  int xpGain,
  int coinGain,
  int atkGain,
  int defGain,
  String& reason,
  int& risk
) {
  risk = 0;


  if (xpGain < 0 || coinGain < 0 || atkGain < 0 || defGain < 0) {
    reason = "Negative gain is not allowed";
    risk = 30;
    return false;
  }


  if (eventType == "streetpass") {
    if (millis() - player.lastPassMs < 3000UL) {
      reason = "StreetPass cooldown violation";
      risk = 15;
      return false;
    }


    if (xpGain > 50 || coinGain > 20) {
      reason = "StreetPass reward too high";
      risk = 25;
      return false;
    }
  }


  if (eventType == "adventure") {
    if (xpGain > 120 || coinGain > 60) {
      reason = "Adventure reward too high";
      risk = 25;
      return false;
    }
  }


  if (eventType == "battle") {
    if (xpGain > 150 || coinGain > 80) {
      reason = "Battle reward too high";
      risk = 25;
      return false;
    }
  }


  int newXP = player.xp + xpGain;
  int newCoins = player.coins + coinGain;
  int newLevel = expectedLevelFromXP(newXP);
  int newAtk = player.atk + atkGain;
  int newDef = player.def + defGain;

  // too fake
  if (newCoins > 9999) {
    reason = "Coin value exceeds maximum allowed range";
    risk = 40;
    return false;
  }


  if (newLevel > 50) {
    reason = "Level exceeds demo maximum";
    risk = 40;
    return false;
  }


  if (newAtk > maxAtkForLevel(newLevel)) {
    reason = "ATK value too high for current level";
    risk = 35;
    return false;
  }


  if (newDef > maxDefForLevel(newLevel)) {
    reason = "DEF value too high for current level";
    risk = 35;
    return false;
  }


  reason = "Event passed validation";
  return true;
}


// make an apply func
bool applyGameEvent(const String& eventType, int xpGain, int coinGain, int atkGain, int defGain) {
  String reason;
  int risk = 0;


  bool ok = validateGameEvent(eventType, xpGain, coinGain, atkGain, defGain, reason, risk);


  if (!ok) {
    addRisk(risk);
    recordAntiCheat(eventType, "warning", reason, false);
    return false;
  }


  player.xp += xpGain;
  player.coins += coinGain;
  player.level = expectedLevelFromXP(player.xp);
  player.atk += atkGain;
  player.def += defGain;


  if (eventType == "streetpass") {
    player.passCount++;
    player.lastPassMs = millis();
  } else if (eventType == "adventure") {
    player.adventureCount++;
  } else if (eventType == "battle") {
    player.battleCount++;
  }


  recordAntiCheat(eventType, "info", reason, true);
  return true;
}

// game api demo
void handleGameStatus() {
  String json = "{";
  json += "\"player_id\":\"" + jsonEscape(player.playerId) + "\",";
  json += "\"level\":" + String(player.level) + ",";
  json += "\"xp\":" + String(player.xp) + ",";
  json += "\"coins\":" + String(player.coins) + ",";
  json += "\"hp\":" + String(player.hp) + ",";
  json += "\"atk\":" + String(player.atk) + ",";
  json += "\"def\":" + String(player.def) + ",";
  json += "\"pass_count\":" + String(player.passCount) + ",";
  json += "\"adventure_count\":" + String(player.adventureCount) + ",";
  json += "\"battle_count\":" + String(player.battleCount) + ",";
  json += "\"risk_score\":" + String(player.riskScore);
  json += "}";


  server.send(200, "application/json", json);
}

// anti cheat log api to find out what happened later
void handleAntiCheatStatus() {
  String json = "{";
  json += "\"risk_score\":" + String(player.riskScore) + ",";
  json += "\"logs\":[";


  for (int i = 0; i < cheatLogCount; i++) {
    int idx = (cheatLogHead - 1 - i + CHEAT_LOG_SIZE) % CHEAT_LOG_SIZE;


    if (i > 0) json += ",";


    json += "{";
    json += "\"event_type\":\"" + jsonEscape(cheatLogs[idx].eventType) + "\",";
    json += "\"severity\":\"" + jsonEscape(cheatLogs[idx].severity) + "\",";
    json += "\"reason\":\"" + jsonEscape(cheatLogs[idx].reason) + "\",";
    json += "\"accepted\":" + String(cheatLogs[idx].accepted ? "true" : "false") + ",";
    json += "\"ms\":" + String(cheatLogs[idx].ms);
    json += "}";
  }


  json += "]";
  json += "}";


  server.send(200, "application/json", json);
}

// ========================= DEMO ZONE
// just for fun lol
void handleSimStreetPass() {
  bool ok = applyGameEvent("streetpass", 25, 5, 0, 0);


  if (ok) {
    server.send(200, "application/json", "{\"ok\":true,\"message\":\"StreetPass simulated\"}");
  } else {
    server.send(409, "application/json", "{\"ok\":false,\"message\":\"StreetPass blocked by anti-cheat\"}");
  }
}

// use in dashboard
void handleSimAdventure() {
  bool ok = applyGameEvent("adventure", 60, 15, 1, 1);


  if (ok) {
    server.send(200, "application/json", "{\"ok\":true,\"message\":\"Adventure simulated\"}");
  } else {
    server.send(409, "application/json", "{\"ok\":false,\"message\":\"Adventure blocked by anti-cheat\"}");
  }
}

// fake battle
void handleSimBattle() {
  bool ok = applyGameEvent("battle", 80, 25, 1, 0);


  if (ok) {
    server.send(200, "application/json", "{\"ok\":true,\"message\":\"Battle simulated\"}");
  } else {
    server.send(409, "application/json", "{\"ok\":false,\"message\":\"Battle blocked by anti-cheat\"}");
  }
}




void handleSimCheat() {
  bool ok = applyGameEvent("manual_state_write", 5000, 9999, 99, 99);


  if (ok) {
    server.send(200, "application/json", "{\"ok\":true,\"message\":\"Cheat accidentally accepted\"}");
  } else {
    server.send(409, "application/json", "{\"ok\":false,\"message\":\"Cheat blocked by anti-cheat\"}");
  }
}


// reset everything
void handleResetGame() {
  player.level = 1;
  player.xp = 0;
  player.coins = 20;
  player.hp = 100;
  player.atk = 10;
  player.def = 5;
  player.passCount = 0;
  player.adventureCount = 0;
  player.battleCount = 0;
  player.riskScore = 0;
  player.lastPassMs = 0;


  cheatLogHead = 0;
  cheatLogCount = 0;


  recordAntiCheat("system", "info", "Game state reset", true);


  server.send(200, "application/json", "{\"ok\":true,\"message\":\"Game reset\"}");
}
// =============================== DEMO ZONE END


// set ip function
void setup() {
  Serial.begin(115200);
  delay(1000);


  // better stability for AP + STA
  WiFi.persistent(false);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);


  // AP Start
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ap_ssid, ap_password);


  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());


  // start connect to wifi (NON-BLOCKING)
  connectSTAAsync();


  server.on("/", handleRoot);
  server.on("/api/status", handleStatus);
  server.on("/api/wifi", HTTP_POST, handleWifiConfig);
  server.on("/api/game", handleGameStatus);
  server.on("/api/anticheat", handleAntiCheatStatus);
  server.on("/api/sim/pass", HTTP_POST, handleSimStreetPass);
  server.on("/api/sim/adventure", HTTP_POST, handleSimAdventure);
  server.on("/api/sim/battle", HTTP_POST, handleSimBattle);
  server.on("/api/sim/cheat", HTTP_POST, handleSimCheat);
  server.on("/api/sim/reset", HTTP_POST, handleResetGame);

  server.onNotFound(handleNotFound);
  server.begin();


  Serial.println("HTTP Server Started");
}


// def main
void loop() {
  server.handleClient(); // don't block client when wifi not ok
  refreshDataIfNeeded();
  delay(2);   // tiny breathing room
}
