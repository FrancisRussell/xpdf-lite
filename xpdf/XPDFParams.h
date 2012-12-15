#ifndef XPDF_PARAMS_H
#define XPDF_PARAMS_H

#include <poppler-config.h>
#include <gtypes.h>
#include <tr1/memory>
#include <GooMutex.h>

class GooList;
class GooString;
class GlobalParams;

class XPDFParams {
public:
  XPDFParams(const char* configPath);
  ~XPDFParams();

  // Will probably become poppler functionality
  
  // SplashOutputDev.cc
  GBool setEnableT1lib(char *s);
  GBool getEnableT1lib();
  // PSOutputDev.cc
  GBool setPSPaperSize(char *size);
  int getPSPaperWidth();
  void setPSPaperWidth(int width);
  int getPSPaperHeight();
  void setPSPaperHeight(int height);


  // Xpdf-specific functionality

  GBool getContinuousView();
  void setContinuousView(GBool cont);
  GooString *getInitialZoom();
  void setInitialZoom(char *s);

  GooString *getLaunchCommand();
  GooString *getURLCommand();
  GooString *getMovieCommand();
  GooList *getKeyBinding(int code, int mods, int context);
  GooString *getPSFile();
  void setPSFile(char *file);
  GBool getPSCrop();
  void setPSCrop(GBool crop);
  
private:
  GooMutex mutex;
  void loadConfig(const char *configPath);
  void parseFile(GooString *fileName, FILE *f);
  void parseLine(char *buf, GooString *fileName, int line);
  void parsePSFile(GooList *tokens, GooString *fileName, int line);
  void parsePSPaperSize(GooList *tokens, GooString *fileName, int line);
  void parsePSImageableArea(GooList *tokens, GooString *fileName, int line);
  void parseInitialZoom(GooList *tokens, GooString *fileName, int line);
  void parseCommand(const char *cmdName, 
         std::tr1::shared_ptr<GooString>& val, GooList *tokens, 
         GooString *fileName, int line);
  void parseBind(GooList *tokens, GooString *fileName, int line);
  void parseUnbind(GooList *tokens, GooString *fileName, int line);
  GBool parseKey(GooString *modKeyStr, GooString *contextStr,
    int *code, int *mods, int *context, const char *cmdName, 
    GooList *tokens, GooString *fileName, int line);
  void parsePSLevel(GooList *tokens, GooString *fileName, int line);
  void parseScreenType(GooList *tokens, GooString *fileName, int line);
  void parseTextEncoding(GooList *tokens, GooString *fileName, int line);
  void parseTextEOL(GooList *tokens, GooString *fileName, int line);
  void parseFontFile(GooList *tokens, GooString *fileName, int line); 

  // xpdf-specific
  GBool continuousView;                          // continuous view mode
  std::tr1::shared_ptr<GooString> launchCommand; // command executed for 'launch' links
  std::tr1::shared_ptr<GooString> urlCommand;    // command executed for URL links
  std::tr1::shared_ptr<GooString> movieCommand;  // command executed for movie annotations
  std::tr1::shared_ptr<GooString> initialZoom;   // initial zoom level
  std::tr1::shared_ptr<GooString> psFile;        // PostScript file or command (for xpdf)
  GooList *keyBindings;                          // key & mouse button bindings [KeyBinding]
  GBool psCrop;                                  // crop PS output to CropBox

  void createDefaultKeyBindings();

  // Likely to appear in poppler
  GBool enableT1lib;		// t1lib enable flag
  int psPaperWidth;		// paper size, in PostScript points, for
  int psPaperHeight;		//   PostScript output
  int psImageableLLX,		// imageable area, in PostScript points,
      psImageableLLY,		//   for PostScript output
      psImageableURX,
      psImageableURY;

  // Present in poppler, but preferable as static utility methods
  static void parseYesNo(const char *cmdName, GBool *flag,
    GooList *tokens, GooString *fileName, int line);
  static GBool parseYesNo2(char *token, GBool *flag);
  static void parseFloat(const char *cmdName, double *val,
    GooList *tokens, GooString *fileName, int line);
  static void parseInteger(const char *cmdName, int *val,
    GooList *tokens, GooString *fileName, int line);

  // For parsing values passed to poppler's GlobalParams
  static void parseYesNo(const char *cmdName, 
    void (GlobalParams::*setter)(bool), GooList *tokens, 
    GooString *fileName, int line);
  static void parseYesNo(const char *cmdName,
    GBool (GlobalParams::*setter)(char*), GooList *tokens, 
    GooString *fileName, int line);
  static void parseFloat(const char *cmdName, 
    void (GlobalParams::*setter)(double),
    GooList *tokens, GooString *fileName, int line);
  static void parseInteger(const char *cmdName,
      void (GlobalParams::*setter)(int), GooList *tokens, 
      GooString *fileName, int line);
};


extern XPDFParams *xpdfParams;

#endif
