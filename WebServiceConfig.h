void notFound(AsyncWebServerRequest *request) {
  String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
  Serial.println(logmessage);
  request->send(404, "text/plain", "Not found");
}

// used by server.on functions to discern whether a user has the correct httpapitoken OR is authenticated by username and password
bool checkUserWebAuth(AsyncWebServerRequest * request) {
  bool isAuthenticated = false;

  if (request->authenticate(config.httpuser.c_str(), config.httppassword.c_str())) {
    Serial.println("is authenticated via username and password");
    isAuthenticated = true;
  }
  return isAuthenticated;
} // checkUserWebAuth

// handles uploads to the filserver
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  // make sure authenticated before allowing upload
  if (checkUserWebAuth(request)) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    Serial.println(logmessage);

    if (!index) {
      logmessage = "Upload Start: " + String(filename);
      // open the file on first call and store the file handle in the request object
      request->_tempFile = SPIFFS.open("/" + filename, "w");
      Serial.println(logmessage);
    }

    if (len) {
      // stream the incoming chunk to the opened file
      request->_tempFile.write(data, len);
      logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
      Serial.println(logmessage);
    }

    if (final) {
      logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
      // close the file handle as the upload is now done
      request->_tempFile.close();
      Serial.println(logmessage);
      request->redirect("/");
    }
  } else {
    Serial.println("Auth: Failed");
    return request->requestAuthentication();
  }
} // handleUpload

// Make size of files human readable
// source: https://github.com/CelliesProjects/minimalUploadAuthESP32
String humanReadableSize(const size_t bytes) {
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + " MB";
  else return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
} // humanReadableSize

// parses and processes webpages
// if the webpage has %SOMETHING% or %SOMETHINGELSE% it will replace those strings with the ones defined
String processor(const String& var) {
  if (var == "FIRMWARE") {
    return FIRMWARE_VERSION;
  }

  if (var == "FREESPIFFS") {
    return humanReadableSize((SPIFFS.totalBytes() - SPIFFS.usedBytes()));
  }

  if (var == "USEDSPIFFS") {
    return humanReadableSize(SPIFFS.usedBytes());
  }

  if (var == "TOTALSPIFFS") {
    return humanReadableSize(SPIFFS.totalBytes());
  }

  if (var == "MODE") {
    switch(mode){
      case 0:
        return modedesc0;
        break;
      case 1:
        return modedesc1;
        break;
    }
    return "unknown";
  }
} // processor

// list all of the files, if ishtml=true, return html rather than simple text
String listFiles(bool ishtml) {
  String returnText = "";
  Serial.println("Listing files stored on SPIFFS");
  File root = SPIFFS.open("/");
  File foundfile = root.openNextFile();
  if (ishtml) {
    returnText += "<table><tr><th align='left'>Name</th><th align='left'>Size</th><th></th><th></th></tr>";
  }
  while (foundfile) {
    if (ishtml) {
      returnText += "<tr align='left'><td>" + String(foundfile.name()) + "</td><td>" + humanReadableSize(foundfile.size()) + "</td>";
      returnText += "<td><button onclick=\"downloadDeleteButton(\'" + String(foundfile.name()) + "\', \'download\')\">Download</button>";
      returnText += "<td><button onclick=\"downloadDeleteButton(\'" + String(foundfile.name()) + "\', \'delete\')\">Delete</button></tr>";
    } else {
      returnText += "File: " + String(foundfile.name()) + " Size: " + humanReadableSize(foundfile.size()) + "\n";
    }
    foundfile = root.openNextFile();
  }
  if (ishtml) {
    returnText += "</table>";
  }
  root.close();
  foundfile.close();
  return returnText;
} // listFiles

String changeMode(bool ishtml) {
  String returnText = "";
  mode = 1 - mode;
  if (ishtml) {
    returnText += "<p>Current mode: ";
    switch(mode){
      case 0:
        returnText += modedesc0;
        break;
      case 1:
        returnText += modedesc1;
    }
    returnText += "</p>";
  }
} // changeMode

void setupWebserver() {
  if (nowifi == false) {
    // configure web server
    config.httpuser = default_httpuser;
    config.httppassword = default_httppassword;
    config.webserverporthttp = default_webserverporthttp;

    Serial.println("Configuring Webserver ...");
    server = new AsyncWebServer(config.webserverporthttp);
    
    // if url isn't found
    server->onNotFound(notFound);
  
    // run handleUpload function when any file is uploaded
    server->onFileUpload(handleUpload);
  
    // visiting this page will cause you to be logged out
    server->on("/logout", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->requestAuthentication();
      request->send(401);
    });
  
    // presents a "you are now logged out webpage
    server->on("/logged-out", HTTP_GET, [](AsyncWebServerRequest * request) {
      String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
      Serial.println(logmessage);
      request->send_P(401, "text/html", logout_html, processor);
    });
  
    server->on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
      String logmessage = "Client:" + request->client()->remoteIP().toString() + + " " + request->url();
  
      if (checkUserWebAuth(request)) {
        logmessage += " Auth: Success";
        Serial.println(logmessage);
        request->send_P(200, "text/html", index_html, processor);
      } else {
        logmessage += " Auth: Failed";
        Serial.println(logmessage);
        return request->requestAuthentication();
      }
  
    });
  
    server->on("/reboot", HTTP_GET, [](AsyncWebServerRequest * request) {
      String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
  
      if (checkUserWebAuth(request)) {
        request->send(200, "text/html", reboot_html);
        logmessage += " Auth: Success";
        Serial.println(logmessage);
        shouldReboot = true;
      } else {
        logmessage += " Auth: Failed";
        Serial.println(logmessage);
        return request->requestAuthentication();
      }
    });
  
    server->on("/listfiles", HTTP_GET, [](AsyncWebServerRequest * request)
    {
      String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
      if (checkUserWebAuth(request)) {
        logmessage += " Auth: Success";
        Serial.println(logmessage);
        request->send(200, "text/plain", listFiles(true));
      } else {
        logmessage += " Auth: Failed";
        Serial.println(logmessage);
        return request->requestAuthentication();
      }
    });
  
    server->on("/mode", HTTP_GET, [](AsyncWebServerRequest * request) {
      String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
  
      if (checkUserWebAuth(request)) {
        request->send(200, "text/html", changeMode(true));
        logmessage += " Auth: Success";
        Serial.println(logmessage);
      } else {
        logmessage += " Auth: Failed";
        Serial.println(logmessage);
        return request->requestAuthentication();
      }
    });
  
  
    server->on("/file", HTTP_GET, [](AsyncWebServerRequest * request) {
      String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
      if (checkUserWebAuth(request)) {
        logmessage += " Auth: Success";
        Serial.println(logmessage);
  
        if (request->hasParam("name") && request->hasParam("action")) {
          const char *fileName = request->getParam("name")->value().c_str();
          const char *fileAction = request->getParam("action")->value().c_str();
  
          logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url() + "?name=" + String(fileName) + "&action=" + String(fileAction);
  
          if (!SPIFFS.exists(fileName)) {
            Serial.println(logmessage + " ERROR: file does not exist");
            request->send(400, "text/plain", "ERROR: file does not exist");
          } else {
            Serial.println(logmessage + " file exists");
            if (strcmp(fileAction, "download") == 0) {
              logmessage += " downloaded";
              request->send(SPIFFS, fileName, "application/octet-stream");
            } else if (strcmp(fileAction, "delete") == 0) {
              logmessage += " deleted";
              SPIFFS.remove(fileName);
              request->send(200, "text/plain", "Deleted File: " + String(fileName));
            } else {
              logmessage += " ERROR: invalid action param supplied";
              request->send(400, "text/plain", "ERROR: invalid action param supplied");
            }
            Serial.println(logmessage);
          }
        } else {
          request->send(400, "text/plain", "ERROR: name and action params required");
        }
      } else {
        logmessage += " Auth: Failed";
        Serial.println(logmessage);
        return request->requestAuthentication();
      }
    });  
    // startup web server
    Serial.println("Starting Webserver ...");
    server->begin();
  }
}
