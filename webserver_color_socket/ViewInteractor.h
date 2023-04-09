#include <FS.h>
#define DBG_OUTPUT_PORT Serial
#include <ESP8266WebServer.h>



class  ViewInteractor {

  public:
  
    void lookupFiles();
    String getContentType(String filename);
    bool isFileRead(String path);
    File getFileRead(String path);
    void handleRoot();
    void serverOnnNotFound(std::unique_ptr<ESP8266WebServer> server);

  private:

    std::unique_ptr<ESP8266WebServer> server;
};
