//========================================================================
//
// GlobalParams.h
//
// Copyright 2001-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef GLOBALPARAMS_H
#define GLOBALPARAMS_H

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include <stdio.h>
#include "gtypes.h"
#include "CharTypes.h"
#include "config.h"

#if MULTITHREADED
#include "GooMutex.h"
#endif

class GooString;
class GooList;
class GooHash;
class NameToCharCode;
class CharCodeToUnicode;
class CharCodeToUnicodeCache;
class UnicodeMap;
class UnicodeMapCache;
class CMap;
class CMapCache;
struct XpdfSecurityHandler;
class GlobalParams;
class SysFontList;

//------------------------------------------------------------------------

// The global parameters object.
extern GlobalParams *globalParams;

//------------------------------------------------------------------------

enum SysFontType {
  sysFontPFA,
  sysFontPFB,
  sysFontTTF,
  sysFontTTC
};

//------------------------------------------------------------------------

class PSFontParam16 {
public:

  GooString *name;		// PDF font name for psResidentFont16;
				//   char collection name for psResidentFontCC
  int wMode;			// writing mode (0=horiz, 1=vert)
  GooString *psFontName;		// PostScript font name
  GooString *encoding;		// encoding

  PSFontParam16(GooString *nameA, int wModeA,
		GooString *psFontNameA, GooString *encodingA);
  ~PSFontParam16();
};

//------------------------------------------------------------------------

enum PSLevel {
  psLevel1,
  psLevel1Sep,
  psLevel2,
  psLevel2Sep,
  psLevel3,
  psLevel3Sep
};

//------------------------------------------------------------------------

enum EndOfLineKind {
  eolUnix,			// LF
  eolDOS,			// CR+LF
  eolMac			// CR
};

//------------------------------------------------------------------------

enum ScreenType {
  screenUnset,
  screenDispersed,
  screenClustered,
  screenStochasticClustered
};

//------------------------------------------------------------------------

class KeyBinding {
public:

  int code;			// 0x20 .. 0xfe = ASCII,
				//   >=0x10000 = special keys, mouse buttons,
				//   etc. (xpdfKeyCode* symbols)
  int mods;			// modifiers (xpdfKeyMod* symbols, or-ed
				//   together)
  int context;			// context (xpdfKeyContext* symbols, or-ed
				//   together)
  GooList *cmds;			// list of commands [GooString]

  KeyBinding(int codeA, int modsA, int contextA, const char *cmd0);
  KeyBinding(int codeA, int modsA, int contextA,
	     const char *cmd0, const char *cmd1);
  KeyBinding(int codeA, int modsA, int contextA, GooList *cmdsA);
  ~KeyBinding();
};

#define xpdfKeyCodeTab            0x1000
#define xpdfKeyCodeReturn         0x1001
#define xpdfKeyCodeEnter          0x1002
#define xpdfKeyCodeBackspace      0x1003
#define xpdfKeyCodeInsert         0x1004
#define xpdfKeyCodeDelete         0x1005
#define xpdfKeyCodeHome           0x1006
#define xpdfKeyCodeEnd            0x1007
#define xpdfKeyCodePgUp           0x1008
#define xpdfKeyCodePgDn           0x1009
#define xpdfKeyCodeLeft           0x100a
#define xpdfKeyCodeRight          0x100b
#define xpdfKeyCodeUp             0x100c
#define xpdfKeyCodeDown           0x100d
#define xpdfKeyCodeF1             0x1100
#define xpdfKeyCodeF35            0x1122
#define xpdfKeyCodeMousePress1    0x2001
#define xpdfKeyCodeMousePress2    0x2002
#define xpdfKeyCodeMousePress3    0x2003
#define xpdfKeyCodeMousePress4    0x2004
#define xpdfKeyCodeMousePress5    0x2005
#define xpdfKeyCodeMousePress6    0x2006
#define xpdfKeyCodeMousePress7    0x2007
// ...
#define xpdfKeyCodeMousePress32   0x2020
#define xpdfKeyCodeMouseRelease1  0x2101
#define xpdfKeyCodeMouseRelease2  0x2102
#define xpdfKeyCodeMouseRelease3  0x2103
#define xpdfKeyCodeMouseRelease4  0x2104
#define xpdfKeyCodeMouseRelease5  0x2105
#define xpdfKeyCodeMouseRelease6  0x2106
#define xpdfKeyCodeMouseRelease7  0x2107
// ...
#define xpdfKeyCodeMouseRelease32 0x2120
#define xpdfKeyModNone            0
#define xpdfKeyModShift           (1 << 0)
#define xpdfKeyModCtrl            (1 << 1)
#define xpdfKeyModAlt             (1 << 2)
#define xpdfKeyContextAny         0
#define xpdfKeyContextFullScreen  (1 << 0)
#define xpdfKeyContextWindow      (2 << 0)
#define xpdfKeyContextContinuous  (1 << 2)
#define xpdfKeyContextSinglePage  (2 << 2)
#define xpdfKeyContextOverLink    (1 << 4)
#define xpdfKeyContextOffLink     (2 << 4)
#define xpdfKeyContextOutline     (1 << 6)
#define xpdfKeyContextMainWin     (2 << 6)
#define xpdfKeyContextScrLockOn   (1 << 8)
#define xpdfKeyContextScrLockOff  (2 << 8)

//------------------------------------------------------------------------

class GlobalParams {
public:

  // Initialize the global parameters by attempting to read a config
  // file.
  GlobalParams(char *cfgFileName);

  ~GlobalParams();

  void setBaseDir(char *dir);
  void setupBaseFonts(char *dir);

  void parseLine(char *buf, GooString *fileName, int line);

  //----- accessors

  CharCode getMacRomanCharCode(char *charName);

  GooString *getBaseDir();
  Unicode mapNameToUnicode(const char *charName);
  UnicodeMap *getResidentUnicodeMap(GooString *encodingName);
  FILE *getUnicodeMapFile(GooString *encodingName);
  FILE *findCMapFile(GooString *collection, GooString *cMapName);
  FILE *findToUnicodeFile(GooString *name);
  GooString *findFontFile(GooString *fontName);
  GooString *findSystemFontFile(GooString *fontName, SysFontType *type,
			      int *fontNum);
  GooString *findCCFontFile(GooString *collection);
  GooString *getPSFile();
  int getPSPaperWidth();
  int getPSPaperHeight();
  void getPSImageableArea(int *llx, int *lly, int *urx, int *ury);
  GBool getPSDuplex();
  GBool getPSCrop();
  GBool getPSExpandSmaller();
  GBool getPSShrinkLarger();
  GBool getPSCenter();
  PSLevel getPSLevel();
  GooString *getPSResidentFont(GooString *fontName);
  GooList *getPSResidentFonts();
  PSFontParam16 *getPSResidentFont16(GooString *fontName, int wMode);
  PSFontParam16 *getPSResidentFontCC(GooString *collection, int wMode);
  GBool getPSEmbedType1();
  GBool getPSEmbedTrueType();
  GBool getPSEmbedCIDPostScript();
  GBool getPSEmbedCIDTrueType();
  GBool getPSFontPassthrough();
  GBool getPSPreload();
  GBool getPSOPI();
  GBool getPSASCIIHex();
  GBool getPSUncompressPreloadedImages();
  double getPSRasterResolution();
  GBool getPSRasterMono();
  GBool getPSAlwaysRasterize();
  GooString *getTextEncodingName();
  EndOfLineKind getTextEOL();
  GBool getTextPageBreaks();
  GBool getTextKeepTinyChars();
  GooString *getInitialZoom();
  GBool getContinuousView();
  GBool getEnableT1lib();
  GBool getEnableFreeType();
  GBool getDisableFreeTypeHinting();
  GBool getAntialias();
  GBool getVectorAntialias();
  GBool getAntialiasPrinting();
  GBool getStrokeAdjust();
  ScreenType getScreenType();
  int getScreenSize();
  int getScreenDotRadius();
  double getScreenGamma();
  double getScreenBlackThreshold();
  double getScreenWhiteThreshold();
  double getMinLineWidth();
  GBool getDrawAnnotations();
  GBool getOverprintPreview() { return overprintPreview; }
  GooString *getLaunchCommand() { return launchCommand; }
  GooString *getURLCommand() { return urlCommand; }
  GooString *getMovieCommand() { return movieCommand; }
  GBool getMapNumericCharNames();
  GBool getMapUnknownCharNames();
  GooList *getKeyBinding(int code, int mods, int context);
  GBool getPrintCommands();
  GBool getErrQuiet();

  CharCodeToUnicode *getCIDToUnicode(GooString *collection);
  CharCodeToUnicode *getUnicodeToUnicode(GooString *fontName);
  UnicodeMap *getUnicodeMap(GooString *encodingName);
  CMap *getCMap(GooString *collection, GooString *cMapName);
  UnicodeMap *getTextEncoding();

  //----- functions to set parameters

  void addFontFile(GooString *fontName, GooString *path);
  void setPSFile(char *file);
  GBool setPSPaperSize(char *size);
  void setPSPaperWidth(int width);
  void setPSPaperHeight(int height);
  void setPSImageableArea(int llx, int lly, int urx, int ury);
  void setPSDuplex(GBool duplex);
  void setPSCrop(GBool crop);
  void setPSExpandSmaller(GBool expand);
  void setPSShrinkLarger(GBool shrink);
  void setPSCenter(GBool center);
  void setPSLevel(PSLevel level);
  void setPSEmbedType1(GBool embed);
  void setPSEmbedTrueType(GBool embed);
  void setPSEmbedCIDPostScript(GBool embed);
  void setPSEmbedCIDTrueType(GBool embed);
  void setPSFontPassthrough(GBool passthrough);
  void setPSPreload(GBool preload);
  void setPSOPI(GBool opi);
  void setPSASCIIHex(GBool hex);
  void setTextEncoding(char *encodingName);
  GBool setTextEOL(char *s);
  void setTextPageBreaks(GBool pageBreaks);
  void setTextKeepTinyChars(GBool keep);
  void setInitialZoom(char *s);
  void setContinuousView(GBool cont);
  GBool setEnableT1lib(char *s);
  GBool setEnableFreeType(char *s);
  GBool setAntialias(char *s);
  GBool setVectorAntialias(char *s);
  void setScreenType(ScreenType t);
  void setScreenSize(int size);
  void setScreenDotRadius(int r);
  void setScreenGamma(double gamma);
  void setScreenBlackThreshold(double thresh);
  void setScreenWhiteThreshold(double thresh);
  void setMapNumericCharNames(GBool map);
  void setMapUnknownCharNames(GBool map);
  void setPrintCommands(GBool printCommandsA);
  void setErrQuiet(GBool errQuietA);

  //----- security handlers

  void addSecurityHandler(XpdfSecurityHandler *handler);
  XpdfSecurityHandler *getSecurityHandler(char *name);

private:

  void createDefaultKeyBindings();
  void parseFile(GooString *fileName, FILE *f);
  void parseNameToUnicode(GooList *tokens, GooString *fileName, int line);
  void parseCIDToUnicode(GooList *tokens, GooString *fileName, int line);
  void parseUnicodeToUnicode(GooList *tokens, GooString *fileName, int line);
  void parseUnicodeMap(GooList *tokens, GooString *fileName, int line);
  void parseCMapDir(GooList *tokens, GooString *fileName, int line);
  void parseToUnicodeDir(GooList *tokens, GooString *fileName, int line);
  void parseFontFile(GooList *tokens, GooString *fileName, int line);
  void parseFontDir(GooList *tokens, GooString *fileName, int line);
  void parseFontFileCC(GooList *tokens, GooString *fileName,
		       int line);
  void parsePSFile(GooList *tokens, GooString *fileName, int line);
  void parsePSPaperSize(GooList *tokens, GooString *fileName, int line);
  void parsePSImageableArea(GooList *tokens, GooString *fileName, int line);
  void parsePSLevel(GooList *tokens, GooString *fileName, int line);
  void parsePSResidentFont(GooList *tokens, GooString *fileName, int line);
  void parsePSResidentFont16(GooList *tokens, GooString *fileName, int line);
  void parsePSResidentFontCC(GooList *tokens, GooString *fileName, int line);
  void parseTextEncoding(GooList *tokens, GooString *fileName, int line);
  void parseTextEOL(GooList *tokens, GooString *fileName, int line);
  void parseInitialZoom(GooList *tokens, GooString *fileName, int line);
  void parseScreenType(GooList *tokens, GooString *fileName, int line);
  void parseBind(GooList *tokens, GooString *fileName, int line);
  void parseUnbind(GooList *tokens, GooString *fileName, int line);
  GBool parseKey(GooString *modKeyStr, GooString *contextStr,
		 int *code, int *mods, int *context,
		 const char *cmdName,
		 GooList *tokens, GooString *fileName, int line);
  void parseCommand(const char *cmdName, GooString **val,
		    GooList *tokens, GooString *fileName, int line);
  void parseYesNo(const char *cmdName, GBool *flag,
		  GooList *tokens, GooString *fileName, int line);
  GBool parseYesNo2(char *token, GBool *flag);
  void parseInteger(const char *cmdName, int *val,
		    GooList *tokens, GooString *fileName, int line);
  void parseFloat(const char *cmdName, double *val,
		  GooList *tokens, GooString *fileName, int line);
  UnicodeMap *getUnicodeMap2(GooString *encodingName);
#ifdef ENABLE_PLUGINS
  GBool loadPlugin(char *type, char *name);
#endif

  //----- static tables

  NameToCharCode *		// mapping from char name to
    macRomanReverseMap;		//   MacRomanEncoding index

  //----- user-modifiable settings

  GooString *baseDir;		// base directory - for plugins, etc.
  NameToCharCode *		// mapping from char name to Unicode
    nameToUnicode;
  GooHash *cidToUnicodes;		// files for mappings from char collections
				//   to Unicode, indexed by collection name
				//   [GooString]
  GooHash *unicodeToUnicodes;	// files for Unicode-to-Unicode mappings,
				//   indexed by font name pattern [GooString]
  GooHash *residentUnicodeMaps;	// mappings from Unicode to char codes,
				//   indexed by encoding name [UnicodeMap]
  GooHash *unicodeMaps;		// files for mappings from Unicode to char
				//   codes, indexed by encoding name [GooString]
  GooHash *cMapDirs;		// list of CMap dirs, indexed by collection
				//   name [GooList[GooString]]
  GooList *toUnicodeDirs;		// list of ToUnicode CMap dirs [GooString]
  GooHash *fontFiles;		// font files: font name mapped to path
				//   [GooString]
  GooList *fontDirs;		// list of font dirs [GooString]
  GooHash *ccFontFiles;		// character collection font files:
				//   collection name  mapped to path [GooString]
  SysFontList *sysFonts;	// system fonts
  GooString *psFile;		// PostScript file or command (for xpdf)
  int psPaperWidth;		// paper size, in PostScript points, for
  int psPaperHeight;		//   PostScript output
  int psImageableLLX,		// imageable area, in PostScript points,
      psImageableLLY,		//   for PostScript output
      psImageableURX,
      psImageableURY;
  GBool psCrop;			// crop PS output to CropBox
  GBool psExpandSmaller;	// expand smaller pages to fill paper
  GBool psShrinkLarger;		// shrink larger pages to fit paper
  GBool psCenter;		// center pages on the paper
  GBool psDuplex;		// enable duplexing in PostScript?
  PSLevel psLevel;		// PostScript level to generate
  GooHash *psResidentFonts;	// 8-bit fonts resident in printer:
				//   PDF font name mapped to PS font name
				//   [GooString]
  GooList *psResidentFonts16;	// 16-bit fonts resident in printer:
				//   PDF font name mapped to font info
				//   [PSFontParam16]
  GooList *psResidentFontsCC;	// 16-bit character collection fonts
				//   resident in printer: collection name
				//   mapped to font info [PSFontParam16]
  GBool psEmbedType1;		// embed Type 1 fonts?
  GBool psEmbedTrueType;	// embed TrueType fonts?
  GBool psEmbedCIDPostScript;	// embed CID PostScript fonts?
  GBool psEmbedCIDTrueType;	// embed CID TrueType fonts?
  GBool psFontPassthrough;	// pass all fonts through as-is?
  GBool psPreload;		// preload PostScript images and forms into
				//   memory
  GBool psOPI;			// generate PostScript OPI comments?
  GBool psASCIIHex;		// use ASCIIHex instead of ASCII85?
  GBool psUncompressPreloadedImages;  // uncompress all preloaded images
  double psRasterResolution;	// PostScript rasterization resolution (dpi)
  GBool psRasterMono;		// true to do PostScript rasterization
				//   in monochrome (gray); false to do it
				//   in color (RGB/CMYK)
  GBool psAlwaysRasterize;	// force PostScript rasterization
  GooString *textEncoding;	// encoding (unicodeMap) to use for text
				//   output
  EndOfLineKind textEOL;	// type of EOL marker to use for text
				//   output
  GBool textPageBreaks;		// insert end-of-page markers?
  GBool textKeepTinyChars;	// keep all characters in text output
  GooString *initialZoom;		// initial zoom level
  GBool continuousView;		// continuous view mode
  GBool enableT1lib;		// t1lib enable flag
  GBool enableFreeType;		// FreeType enable flag
  GBool disableFreeTypeHinting;	// FreeType hinting disable flag
  GBool antialias;		// font anti-aliasing enable flag
  GBool vectorAntialias;	// vector anti-aliasing enable flag
  GBool antialiasPrinting;	// allow anti-aliasing when printing
  GBool strokeAdjust;		// stroke adjustment enable flag
  ScreenType screenType;	// halftone screen type
  int screenSize;		// screen matrix size
  int screenDotRadius;		// screen dot radius
  double screenGamma;		// screen gamma correction
  double screenBlackThreshold;	// screen black clamping threshold
  double screenWhiteThreshold;	// screen white clamping threshold
  double minLineWidth;		// minimum line width
  GBool drawAnnotations;	// draw annotations or not
  GBool overprintPreview;	// enable overprint preview
  GooString *launchCommand;	// command executed for 'launch' links
  GooString *urlCommand;		// command executed for URL links
  GooString *movieCommand;	// command executed for movie annotations
  GBool mapNumericCharNames;	// map numeric char names (from font subsets)?
  GBool mapUnknownCharNames;	// map unknown char names?
  GooList *keyBindings;		// key & mouse button bindings [KeyBinding]
  GBool printCommands;		// print the drawing commands
  GBool errQuiet;		// suppress error messages?

  CharCodeToUnicodeCache *cidToUnicodeCache;
  CharCodeToUnicodeCache *unicodeToUnicodeCache;
  UnicodeMapCache *unicodeMapCache;
  CMapCache *cMapCache;

#ifdef ENABLE_PLUGINS
  GooList *plugins;		// list of plugins [Plugin]
  GooList *securityHandlers;	// list of loaded security handlers
				//   [XpdfSecurityHandler]
#endif

#if MULTITHREADED
  GooMutex mutex;
  GooMutex unicodeMapCacheMutex;
  GooMutex cMapCacheMutex;
#endif
};

#endif
