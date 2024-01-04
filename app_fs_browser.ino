#include <Arduino.h>
#include <esp_ota_ops.h>

#define BLYNK_TEMPLATE_ID "TMPL20o-ReO3r"
#define BLYNK_TEMPLATE_NAME "switch"
#define BLYNK_AUTH_TOKEN "2wzUwRv1MrP_-OarfCkzZ6R39EC76pa1"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <Preferences.h>
Preferences preferences;

const int ledPin = 2;  // Built-in LED pin on most ESP32 boards

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include "SPIFFS.h"

#include <WebServer.h>
#include <ESPmDNS.h>
const char *host = "esp32fs";
WebServer server(80);

char ssid[50];  // Variable to store the selected WiFi SSID
char pass[50];  // Variable to store the entered WiFi password

void handleRoot() {
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
    html += "<a href='/download' download='data.txt'>";
    html += "<button>Download</button></a>";
    html += "</body></html>";

    server.send(200, "text/html", html);
  } else {
    server.send(404, "text/plain", "FileNotFound");
  }
}

void handleDownload() {
  File file = SPIFFS.open("/data.txt", "r");
  if (file) {
    // Set the content type to plain text
    server.sendHeader("Content-Type", "text/plain");
    // Provide the file for download
    server.send(200, "", file.readString());
    file.close();
  } else {
    server.send(404, "text/plain", "FileNotFound");
  }
}

BlynkTimer timer;

BLYNK_WRITE(V3)
{
  // Set incoming value from pin V6 to a variable
  int switchValue = param.asInt();

  // Check the state of the switch
  if (switchValue == 1)
  {
    Serial.println("Switch is ON");
    digitalWrite(ledPin, HIGH);
    Blynk.virtualWrite(V2, "Switch is ON");
  }
  else
  {
    Serial.println("Switch is OFF");
    digitalWrite(ledPin, LOW);  // Turn off LED
    Blynk.virtualWrite(V2, "Switch is OFF");
  }
}

// This function is called every time the Virtual Pin 0 state changes
BLYNK_WRITE(V0)
{
  // Set incoming value from pin V0 to a variable
  int value = param.asInt();

  // Update state
  Blynk.virtualWrite(V1, value);
}

// This function is called every time the device is connected to the Blynk.Cloud
BLYNK_CONNECTED()
{
  // Change Web Link Button message to "Congratulations!"
  //Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  //Blynk.setProperty(V3, "onImageUrl",  "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V3, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
}

// This function sends Arduino's uptime every second to Virtual Pin 2.
unsigned long lastWriteTime = 0;
const unsigned long writeInterval = 60000; 

void myTimerEvent()
{
  unsigned long currentMillis = millis();

  File file = SPIFFS.open("/data.txt", FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  
  int randomNumber = random(101);
  
  Blynk.virtualWrite(V2, randomNumber);
  Blynk.virtualWrite(V5, millis() / 1000);

// Get the current time
  unsigned long elapsedSeconds = currentMillis / 1000;

  // Format the time as HH:MM:SS
  char timeStr[9];
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", elapsedSeconds / 3600, (elapsedSeconds % 3600) / 60, elapsedSeconds % 60);

  // file.printf("%s, %d\n", timeStr, randomNumber);
  Serial.printf("Serial Write: %s, %d\n", timeStr, randomNumber);

    // Check if the time interval for writing to the file has passed
  if (currentMillis - lastWriteTime >= writeInterval)
  {
    file.printf("File Write: %s, %d\n", timeStr, randomNumber);
    // Update the last write time
    lastWriteTime = currentMillis;
  }

  // Close the file
  file.close();
}


void setup()
{
  // Debug console
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  // Retrieve flash information
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);

  // Print flash information
  //Serial.printf("Flash size: %d bytes\n", spi_flash_get_chip_size());
  //Serial.printf("Free sketch space: %d bytes\n", ESP.getFreeSketchSpace());

  Serial.printf("Flash size: %.2f MB\n", static_cast<float>(spi_flash_get_chip_size()) / (1024 * 1024));
  Serial.printf("Free sketch space: %.2f MB\n", static_cast<float>(ESP.getFreeSketchSpace()) / (1024 * 1024));


  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  
  File file = SPIFFS.open("/data.txt");
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }
  
  // Serial.print("File size: ");
  // Serial.println(file.size());
  Serial.printf("File size: %.4f MB\n", static_cast<float>(file.size()) / (1024 * 1024));

  //Serial.println("File size: %.2f KB\n", static_cast<float>(file.size()) / 1024.0);

  //Serial.println("File Content:");
  //while(file.available()){
  //  Serial.write(file.read());
  //}
  file.close();


    preferences.begin("myApp", false); // Set the namespace to "myApp", don't auto-format
  // Initialize ssid and pass variables to null
  // Initialize ssid and pass variables as empty strings
  // it gets initilziaed here, but not put in memory
  ssid[0] = '\0';
  pass[0] = '\0';

  // Read ssid and pass from preferences
  // this is also propbably 0 length
  preferences.getString("ssid", ssid, sizeof(ssid));
  preferences.getString("pass", pass, sizeof(pass));

    // Check if ssid and pass are both null
  if (strlen(ssid) == 0 && strlen(pass) == 0)
  {
    // Prompt user to choose WiFi network
    Serial.println("Scanning available networks...");
    int numNetworks = WiFi.scanNetworks();
    Serial.println("Available Networks:");
    for (int i = 0; i < numNetworks; ++i) {
      Serial.print(i, DEC);
      Serial.print(". ");
      Serial.println(WiFi.SSID(i));
    }
    
    Serial.println("Enter the number of the desired WiFi network:");
    while (Serial.available() == 0) {
      // Wait for user input
    }
    int selectedNetwork = Serial.parseInt();
    Serial.println("Selected network: " + WiFi.SSID(selectedNetwork));
  
    // Store the selected network's SSID
    strcpy(ssid, WiFi.SSID(selectedNetwork).c_str());
  
    // Prompt user to enter WiFi password
    Serial.println("Enter the WiFi password:");
    while (Serial.available() == 0) {
      // Wait for user input
    }
  
    Serial.readBytesUntil('\n', pass, sizeof(pass));
    pass[strlen(pass)] = '\0';  // Remove newline character
  
    Serial.print("Entered password: ");
    Serial.println(pass);
  
    Serial.println("Connecting to " + String(ssid));
    // Connect to the selected WiFi network
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
    Serial.print("Wi-Fi Status: ");
    Serial.println(WiFi.status());  
  
    bool result = Blynk.connected();
  
    if (result)
    {
      Serial.println("Connected!");
      // it only gets put in memory here.
      preferences.putString("ssid", ssid);
      preferences.putString("pass", pass);
    }
    else
    {
      Serial.println("NOT Connected!");
    }
  
    }
  else
  {  
    //ssid[0] = '\0';
    //pass[0] = '\0';
    //preferences.putString("ssid", ssid);
    //preferences.putString("pass", pass);
    Serial.println("Connecting to " + String(ssid));
    // Connect to the selected WiFi network
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  }

  
  // Setup a function to be called every second
  timer.setInterval(1000L, myTimerEvent);

    // Start MDNS
  if (!MDNS.begin(host)) {
    Serial.println("Error setting up MDNS responder!");
  } else {
    Serial.println("MDNS responder started");
  }



  server.on("/", HTTP_GET, handleRoot);
  server.on("/download", HTTP_GET, handleDownload);

  server.begin();
  Serial.println("HTTP server started");
}

void loop()
{
  Blynk.run();
  timer.run();
  server.handleClient();
  // You can inject your own code or combine it with other sketches.
  // Check other examples on how to communicate with Blynk. Remember
  // to avoid delay() function!
}
