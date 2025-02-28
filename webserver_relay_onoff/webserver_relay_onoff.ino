#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <WiFiManager.h>

const byte interruptPin = 0;
#define LED_R 2
#define DBG_OUTPUT_PORT Serial

const char* ssid = "Ving";
const char* password = "123456789";
const char* host = "khuonvienxanh";

boolean device_one_state = false;

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
int wsClientNumber[5] = { -1, -1, -1, -1, -1};
int lastClientIndex = 0;
const int max_ws_client = 5;
//holds the current upload
File fsUploadFile;
volatile int state = LOW;

void setStatus(boolean st)
{
  if (st)
  {
    device_one_state = true;
    Serial.print("Change state:");
    Serial.println(state);
    digitalWrite(LED_R, LOW); // Xuất trạng thái  ra chân GPIO16
  }
  else
  {
    device_one_state = false;
    Serial.print("Change state:");
    Serial.println(st);
    digitalWrite(LED_R, HIGH); // Xuất trạng thái  ra chân GPIO16
  }
}

//format bytes
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}

String getContentType(String filename) {
  if (server.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path) {
  DBG_OUTPUT_PORT.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void handleFileUpload() {
  if (server.uri() != "/edit") return;
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;
    DBG_OUTPUT_PORT.print("handleFileUpload Name: "); DBG_OUTPUT_PORT.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    //DBG_OUTPUT_PORT.print("handleFileUpload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile)
      fsUploadFile.close();
    DBG_OUTPUT_PORT.print("handleFileUpload Size: "); DBG_OUTPUT_PORT.println(upload.totalSize);
  }
}

void handleFileDelete() {
  if (server.args() == 0) return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  DBG_OUTPUT_PORT.println("handleFileDelete: " + path);
  if (path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if (!SPIFFS.exists(path))
    return server.send(404, "text/plain", "FileNotFound");
  SPIFFS.remove(path);
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileCreate() {
  if (server.args() == 0)
    return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  DBG_OUTPUT_PORT.println("handleFileCreate: " + path);
  if (path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if (SPIFFS.exists(path))
    return server.send(500, "text/plain", "FILE EXISTS");
  File file = SPIFFS.open(path, "w");
  if (file)
    file.close();
  else
    return server.send(500, "text/plain", "CREATE FAILED");
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileList() {
  if (!server.hasArg("dir")) {
    server.send(500, "text/plain", "BAD ARGS");
    return;
  }

  String path = server.arg("dir");
  DBG_OUTPUT_PORT.println("handleFileList: " + path);
  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while (dir.next()) {
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir) ? "dir" : "file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }

  output += "]";
  server.send(200, "text/json", output);
}
void wsSendState()
{
  String json = "{";
  json += "\"devices\":[";//"devices": [
  //-------------------------
  json += "{";//{
  json += "\"state\":";//  "state":
  if (device_one_state) json += "\"on\"";
  else json += "\"off\"";
  json += "}";//}
  json += "]";//}
  json += "}";
  int numcl = 0;
  for (numcl = 0; numcl < max_ws_client; numcl++)
  {
    if (wsClientNumber[numcl] != -1)
      webSocket.sendTXT(wsClientNumber[numcl], json);
  }

  json = String();
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
  switch (type) {
    case WStype_DISCONNECTED:
      DBG_OUTPUT_PORT.printf("[%u] Disconnected!n", num);
      break;
    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        DBG_OUTPUT_PORT.printf("[%u] Connected from %d.%d.%d.%d url: %sn", num, ip[0], ip[1], ip[2], ip[3], payload);

        int index = (num % max_ws_client);
        if (index <= 0) index = 0;
        wsClientNumber[index] = num;
        DBG_OUTPUT_PORT.printf("Save client index %d :%un", index, num);
        // send message to client
        wsSendState();
      }
      break;
    case WStype_TEXT:
      DBG_OUTPUT_PORT.printf("[%u] get Text: %sn", num, payload);
      if (payload[0] == '1') {
        setStatus(true);
        wsSendState();
      }
      else if (payload[0] == '0') {
        setStatus(false);
        wsSendState();
      }

      break;
  }

}
void changestate() {
  state = !state;
  setStatus(state);
  wsSendState();
}
void setup(void) {
  pinMode(interruptPin, INPUT_PULLUP);
  pinMode(LED_R, OUTPUT);
  setStatus(false);
  DBG_OUTPUT_PORT.begin(115200);
  DBG_OUTPUT_PORT.print("n");
  DBG_OUTPUT_PORT.setDebugOutput(true);
  SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      DBG_OUTPUT_PORT.printf("FS File: %s, size: %sn", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    DBG_OUTPUT_PORT.printf("n");
  }


  //WIFI INIT
//  DBG_OUTPUT_PORT.printf("Connecting to %sn", ssid);
//  if (String(WiFi.SSID()) != String(ssid)) {
//    WiFi.begin(ssid, password);
//  }

  setupWiFi();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DBG_OUTPUT_PORT.print(".");
  }
  DBG_OUTPUT_PORT.println("");
  DBG_OUTPUT_PORT.print("Connected! IP address: ");
  DBG_OUTPUT_PORT.println(WiFi.localIP());

  MDNS.begin(host);
  DBG_OUTPUT_PORT.print("Open http://");
  DBG_OUTPUT_PORT.print(host);
  DBG_OUTPUT_PORT.println(".local/edit to see the file browser");

  //SERVER INIT
  //list directory
  server.on("/list", HTTP_GET, handleFileList);
  //load editor
  server.on("/edit", HTTP_GET, []() {
    if (!handleFileRead("/edit.htm")) server.send(404, "text/plain", "FileNotFound");
  });
  //create file
  server.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file
  server.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  server.on("/edit", HTTP_POST, []() {
    server.send(200, "text/plain", "");
  }, handleFileUpload);

  //called when the url is not defined here
  //use it to load content from SPIFFS
  server.onNotFound([]() {
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });

  //get heap status, analog input value and all GPIO statuses in one json call
  server.on("/all", HTTP_GET, []() {
    String json = "{";
    json += "\"heap\":" + String(ESP.getFreeHeap());
    json += ", \"analog\":" + String(analogRead(A0));
    json += ", \"gpio\":" + String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
    json += "}";
    server.send(200, "text/json", json);
    json = String();
  });
  server.begin();
  // start webSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  // Add service to MDNS
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);
  DBG_OUTPUT_PORT.println("HTTP server started");
  attachInterrupt(digitalPinToInterrupt(interruptPin), changestate, FALLING);
}

void loop(void) {
  server.handleClient();
  webSocket.loop();
}

void setupWiFi() {
    delay(10000);

    WiFiManager wifiManager;
     wifiManager.setConfigPortalTimeout(60);
    wifiManager.startConfigPortal();
}
