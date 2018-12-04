#include <FS.h>
#define DBG_OUTPUT_PORT Serial


class  ViewInteractor {
  
    public: 
    void lookupFiles();
    String getContentType(String filename);
    bool isFileRead(String path);
    File getFileRead(String path);
};

