

void setupWifiConfigPortal() {

  Serial.println("Mounting SPIFFS ...");
  if (!SPIFFS.begin(true)) {
    // if you have not used SPIFFS before on a ESP32, it will show this error.
    // after a reboot SPIFFS will be configured and will happily work.
    Serial.println("ERROR: Cannot mount SPIFFS, Rebooting");
    rebootESP("ERROR: Cannot mount SPIFFS, Rebooting");
  }
  
  WiFiSettings.hostname = "LEDMatrix-";
  WiFiSettings.onSuccess  = []() { 
    Serial.println("Success :)"); 
    printArray(Wifigreen, 1000);
  };
  WiFiSettings.onFailure  = []() {
    Serial.println("Failure :(");
    printArray(Wifired, 1000);
  };
  WiFiSettings.onPortalWaitLoop = []() { 
    Serial.print("p"); 
    printArray(Wificonf, 500);
    FastLED.clear();
    delay(500); 
  };

  if (digitalRead(ROTENC_0_SW) == LOW) {
    nowifi = true;
    printArray(Wifinone,2000);
  } else {
    nowifi = false;
    printArray(Wificonf, 0);
    WiFiSettings.connect(true, 30);
    
    Serial.println("\n\nNetwork Configuration:");
    Serial.println("----------------------");
    Serial.print("         SSID: "); Serial.println(WiFi.SSID());
    Serial.print("  Wifi Status: "); Serial.println(WiFi.status());
    Serial.print("Wifi Strength: "); Serial.print(WiFi.RSSI()); Serial.println(" dBm");
    Serial.print("          MAC: "); Serial.println(WiFi.macAddress());
    Serial.print("           IP: "); Serial.println(WiFi.localIP());
    Serial.print("       Subnet: "); Serial.println(WiFi.subnetMask());
    Serial.print("      Gateway: "); Serial.println(WiFi.gatewayIP());
    Serial.print("        DNS 1: "); Serial.println(WiFi.dnsIP(0));
    Serial.print("        DNS 2: "); Serial.println(WiFi.dnsIP(1));
    Serial.print("        DNS 3: "); Serial.println(WiFi.dnsIP(2));
    Serial.println();

    // Show IP address
    IPAddress ip = WiFi.localIP();
    Serial.println(ip.toString());
    Serial.println(sizeof(ip.toString()));
    showLEDText(ip.toString());
  }
  
}
