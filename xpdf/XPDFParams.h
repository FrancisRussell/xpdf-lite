#ifndef XPDF_PARAMS_H
#define XPDF_PARAMS_H

#include <poppler-config.h>
#include <gtypes.h>
#include <tr1/memory>
#include <GooMutex.h>

class GooList;
class GooString;

class XPDFParams {
public:
  XPDFParams();
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
};


extern XPDFParams *xpdfParams;

#endif
