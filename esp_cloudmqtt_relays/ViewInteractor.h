#include <FS.h>
#include <SPIFFS.h>
#include <WiFi.h>
#define DBG_OUTPUT_PORT Serial
#include <WebServer.h>





class  ViewInteractor {

  public:
  
    void lookupFiles();
    String getContentType(String filename);
    bool isFileRead(String path);
    File getFileRead(String path);
    void handleRoot();
    void serverOnnNotFound(std::unique_ptr<WebServer> server);

  private:

    std::unique_ptr<WebServer> server;
};
