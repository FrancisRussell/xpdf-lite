#ifndef XPDF_PARAMS_H
#define XPDF_PARAMS_H

#include <poppler-config.h>
#include <gtypes.h>
#include <GooMutex.h>

class GooList;
class GooString;

class XPDFParams {
public:
  XPDFParams();

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
  GBool continuousView;		// continuous view mode
  GooString *launchCommand;	// command executed for 'launch' links
  GooString *urlCommand;	// command executed for URL links
  GooString *movieCommand;	// command executed for movie annotations
  GooList *keyBindings;		// key & mouse button bindings [KeyBinding]
  GooString *initialZoom;	// initial zoom level
  GooString *psFile;		// PostScript file or command (for xpdf)
  GBool psCrop;			// crop PS output to CropBox

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
