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
IPAddress local_IP(192, 168, 0, 1);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);


// STA ONLINE // set up late in dashboard
const char* sta_ssid = "Home_2.4ghz";      // connect to wifi ssid
const char* sta_password = "lingzhi0916";


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
  "latitude=34.05&longitude=-118.24&current=temperature_2m,weather_code&timezone=auto";


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
String extractJsonValue(const String& src, const String& key) {
  String pattern = "\"" + key + "\":";
  int start = src.indexOf(pattern);
  if (start < 0) return "";
  start += pattern.length();


  while (start < (int)src.length() &&
         (src[start] == ' ' || src[start] == '\n' || src[start] == '\r' || src[start] == '\t')) {
    start++;
  }


  // string num
  if (start < (int)src.length() && src[start] == '"') {
    start++;
    int end = src.indexOf('"', start);
    if (end < 0) return "";
    return src.substring(start, end);
  }


  // number, bool, NULL
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
  return out; // result
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


// start STA connect, NON-BLOCKING
void connectSTAAsync() {
  if (strlen(sta_ssid) == 0) {
    Serial.println("STA SSID empty, skip connect");
    return;
  }


  if (WiFi.status() == WL_CONNECTED) return;


  // retry every 10 sec
  if (millis() - lastSTATryMs < 10000UL) return;


  Serial.print("STA connecting to: ");
  Serial.println(sta_ssid);


  WiFi.begin(sta_ssid, sta_password);
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


// get weather
bool fetchWeatherNow() {
  if (WiFi.status() != WL_CONNECTED) {
    weatherCache = "Wifi Offline";
    return false;
  }


  WiFiClientSecure client;
  client.setInsecure();      // cert later
  client.setTimeout(3000);   // important: avoid long hang


  HTTPClient http;
  http.setConnectTimeout(3000);
  http.setTimeout(3000);


  if (!http.begin(client, weatherUrl)) {
    weatherCache = "HTTP Connect Failed";
    return false;
  }


  // GET request
  int code = http.GET();
  if (code <= 0) {
    weatherCache = "HTTP GET Weather Failed";
    http.end();
    return false;
  }


  if (code != HTTP_CODE_OK) {
    weatherCache = "HTTP " + String(code);
    http.end();
    return false;
  }


  String payload = http.getString();
  http.end();


  String timeVal = extractJsonValue(payload, "time");
  String tempVal = extractJsonValue(payload, "temperature_2m");
  String codeVal = extractJsonValue(payload, "weather_code");


  if (timeVal == "" || tempVal == "" || codeVal == "") {
    weatherCache = "Parse Failed";
    return false;
  }


  tempC = tempVal.toFloat();
  weatherCode = codeVal.toInt();
  weatherCache = weatherCodeToText(weatherCode) + ", " + String(tempC, 1) + " C";
  lastWeatherMs = millis();


  Serial.print("Weather updated: ");
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


// home page
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Dashboard</title>
  <style>
    body { font-family: Arial, sans-serif; background:#f5f5f5; margin:0; padding:24px; }
    .wrap { max-width: 560px; margin: 0 auto; }
    .card {
      background:#fff; border-radius:16px; padding:20px; margin-bottom:16px;
      box-shadow:0 4px 16px rgba(0,0,0,.08);
    }
    h1 { margin:0 0 8px 0; font-size:28px; }
    .label { color:#666; font-size:14px; margin-bottom:6px; }
    .value { font-size:22px; font-weight:700; white-space:pre-line; }
    .muted { color:#777; font-size:13px; white-space:pre-line; }
  </style>
</head>
<body>
  <div class="wrap">
    <div class="card">
      <h1>ESP32 Dashboard</h1>
      <div class="muted">AP + STA / time + weather</div>
    </div>


    <div class="card">
      <div class="label">Current Time</div>
      <div class="value" id="time">Loading...</div>
    </div>


    <div class="card">
      <div class="label">Weather</div>
      <div class="value" id="weather">Loading...</div>
    </div>


    <div class="card">
      <div class="label">Network</div>
      <div class="muted" id="net">Loading...</div>
    </div>
  </div>


  <script>
    async function refresh() {
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
        document.getElementById('net').innerText = "API read failed: " + e;
      }
    }


    refresh();
    setInterval(refresh, 5000);
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


// 404
void handleNotFound() {
  server.send(404, "text/plain", "404 Not Found");
}


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
