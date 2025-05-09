#include <FS.h>
#include <SPIFFS.h>
#define DBG_OUTPUT_PORT Serial
#include <WebServer.h>  // Thay thế ESP8266WebServer bằng WebServer

class ViewInteractor {
  public:
    void lookupFiles();
    String getContentType(String filename);
    bool isFileRead(String path);
    File getFileRead(String path);
    void handleRoot(WebServer *server);
    void serverOnNotFound(std::unique_ptr<WebServer> server);

  private:
    std::unique_ptr<WebServer> server;
};
