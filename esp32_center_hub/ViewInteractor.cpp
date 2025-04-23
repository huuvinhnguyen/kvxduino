#include "ViewInteractor.h"

void ViewInteractor::lookupFiles() {
  if (!SPIFFS.begin(true)) {  // Thay SPIFFS.begin() bằng SPIFFS.begin(true)
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  File root = SPIFFS.open("/");
  if (!root || !root.isDirectory()) {
    Serial.println("Failed to open directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    String fileName = file.name();
    size_t fileSize = file.size();
    Serial.printf("File: %s, Size: %u bytes\n", fileName.c_str(), fileSize);
    file = root.openNextFile();
  }
}

String ViewInteractor::getContentType(String filename) {
  if (filename.endsWith("/")) filename += "index.htm";
  if (filename.endsWith(".htm")) return "text/html";
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

bool ViewInteractor::isFileRead(String path) {
  DBG_OUTPUT_PORT.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.htm";

  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz)) path += ".gz";
    return true;
  }

  return false;
}

File ViewInteractor::getFileRead(String path) {
  File file = SPIFFS.open(path, "r");
  return file;
}

void ViewInteractor::serverOnNotFound(std::unique_ptr<WebServer> server) {
  server->onNotFound([&, &server]() {
    String path = server->uri();
    if (!isFileRead(path)) {
      server->send(404, "text/plain", "FileNotFound");
    } else {
      File file = this->getFileRead(path);
      size_t sent = server->streamFile(file, getContentType(path));
      file.close();
    }
  });
}

void ViewInteractor::handleRoot(WebServer *server) {
  File index = SPIFFS.open("/time.htm", "r");
  if (!index) {
    Serial.println("Failed to open file /time.htm");
    return;
  }

  server->streamFile(index, "text/html");
  index.close();
}
