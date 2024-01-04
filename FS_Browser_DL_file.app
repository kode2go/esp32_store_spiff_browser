#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <SPIFFS.h>

const char* ssid = "x";
const char* password = "x";
const char* host = "esp32fs";
WebServer server(80);

void setup(void) {
  Serial.begin(115200);
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS initialization failed!");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Start MDNS
  if (!MDNS.begin(host)) {
    Serial.println("Error setting up MDNS responder!");
  } else {
    Serial.println("MDNS responder started");
  }

  // Set up server route for displaying file name and download button
  server.on("/", HTTP_GET, []() {
    File file = SPIFFS.open("/data.txt", "r");
    if (file) {
      // Get file content
      String content = "";
      while (file.available()) {
        content += (char)file.read();
      }
      file.close();

      // Send HTML page with file name and download button
      String html = "<html><head></head><body>";
      html += "<h2>File Name: data.txt</h2>";
      html += "<a href='data:text/plain," + content + "' download='data.txt'>";
      html += "<button>Download</button></a>";
      html += "</body></html>";

      server.send(200, "text/html", html);
    } else {
      server.send(404, "text/plain", "FileNotFound");
    }
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  delay(2);
}
