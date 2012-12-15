#include "XPDFParams.h"
#include <gtypes.h>
#include <GooMutex.h>
#include <GooList.h>
#include <GooString.h>
#include <gfile.h>
#include <GlobalParams.h>
#include <Error.h>
#include "KeyBinding.h"
#include "config.h"
#include <cstring>
#include <cstdio>
#include <cctype>
#include <cassert>
#include <cstdlib>
#include <tr1/memory>

#if HAVE_PAPER_H
#include <paper.h>
#endif

#  define lockGlobalParams            gLockMutex(&mutex)
#  define unlockGlobalParams          gUnlockMutex(&mutex)

XPDFParams *xpdfParams = NULL;

XPDFParams::XPDFParams(const char *configPath)
{
  gInitMutex(&mutex);
  continuousView = gFalse;
  initialZoom.reset(new GooString("125"));
  psCrop = gTrue;
  enableT1lib = gTrue;

#if HAVE_PAPER_H
  char *paperName = NULL;
  const struct paper *paperType;
  paperinit();
  if ((paperName = systempapername())) {
    paperType = paperinfo(paperName);
    psPaperWidth = static_cast<int>(paperpswidth(paperType));
    psPaperHeight = static_cast<int>(paperpsheight(paperType));
  } else {
    error(errConfig, -1, "No paper information available - using defaults");
    psPaperWidth = defPaperWidth;
    psPaperHeight = defPaperHeight;
  }
  paperdone();
#else
  psPaperWidth = defPaperWidth;
  psPaperHeight = defPaperHeight;
#endif

  psImageableLLX = psImageableLLY = 0;
  psImageableURX = psPaperWidth;
  psImageableURY = psPaperHeight;

  createDefaultKeyBindings();
  loadConfig(configPath);
}

void XPDFParams::loadConfig(const char* configPath)
{
  // look for a user config file, then a system-wide config file
  FILE *f = NULL;
  GooString *fileName = NULL;
  if (configPath && configPath[0]) {
    fileName = new GooString(configPath);
    if (!(f = fopen(fileName->getCString(), "r"))) {
      delete fileName;
    }
  }
  if (!f) {
    fileName = appendToPath(getHomeDir(), xpdfUserConfigFile);
    if (!(f = fopen(fileName->getCString(), "r"))) {
      delete fileName;
    }
  }
  if (!f) {
#ifdef WIN32
    char buf[512];
    i = GetModuleFileName(NULL, buf, sizeof(buf));
    if (i <= 0 || i >= sizeof(buf)) {
      // error or path too long for buffer - just use the current dir
      buf[0] = '\0';
    }
    fileName = grabPath(buf);
    appendToPath(fileName, xpdfSysConfigFile);
#else
    fileName = new GooString(xpdfSysConfigFile);
#endif
    if (!(f = fopen(fileName->getCString(), "r"))) {
      delete fileName;
    }
  }
  if (f) {
    parseFile(fileName, f);
    delete fileName;
    fclose(f);
  }
}

void XPDFParams::parseFile(GooString *fileName, FILE *f) 
{
  char buf[512];
  int line = 1;

  while (getLine(buf, sizeof(buf) - 1, f)) 
  {
    parseLine(buf, fileName, line);
    ++line;
  }
}

void XPDFParams::parseLine(char *buf, GooString *fileName, int line) 
{
  // break the line into tokens
  GooList *tokens = new GooList();
  char *p1 = buf;
  while (*p1) {
    for (; *p1 && isspace(*p1); ++p1) ;
    if (!*p1) {
      break;
    }
    char *p2;
    if (*p1 == '"' || *p1 == '\'') {
      for (p2 = p1 + 1; *p2 && *p2 != *p1; ++p2) ;
      ++p1;
    } else {
      for (p2 = p1 + 1; *p2 && !isspace(*p2); ++p2) ;
    }
    tokens->append(new GooString(p1, (int)(p2 - p1)));
    p1 = *p2 ? p2 + 1 : p2;
  }

  // parse the line
  if (tokens->getLength() > 0 &&
      ((GooString *)tokens->get(0))->getChar(0) != '#') {
    GooString *cmd = (GooString *)tokens->get(0);
    if (!cmd->cmp("include")) {
      if (tokens->getLength() == 2) {
        FILE *f2;
	GooString *incFile = (GooString *)tokens->get(1);
	if (f2 = openFile(incFile->getCString(), "r")) {
	  parseFile(incFile, f2);
	  fclose(f2);
	} else {
	  error(errConfig, -1,
		"Couldn't find included config file: '{0:t}' ({1:t}:{2:d})",
		incFile, fileName, line);
	}
      } else {
	error(errConfig, -1, "Bad 'include' config file command ({0:t}:{1:d})",
	      fileName, line);
      }
    } else if (!cmd->cmp("nameToUnicode")) {
      //parseNameToUnicode(tokens, fileName, line);
    } else if (!cmd->cmp("cidToUnicode")) {
      //parseCIDToUnicode(tokens, fileName, line);
    } else if (!cmd->cmp("unicodeToUnicode")) {
      //parseUnicodeToUnicode(tokens, fileName, line);
    } else if (!cmd->cmp("unicodeMap")) {
      //parseUnicodeMap(tokens, fileName, line);
    } else if (!cmd->cmp("cMapDir")) {
      //parseCMapDir(tokens, fileName, line);
    } else if (!cmd->cmp("toUnicodeDir")) {
      //parseToUnicodeDir(tokens, fileName, line);
    } else if (!cmd->cmp("fontFile")) {
      //parseFontFile(tokens, fileName, line);
    } else if (!cmd->cmp("fontDir")) {
      //parseFontDir(tokens, fileName, line);
    } else if (!cmd->cmp("fontFileCC")) {
      //parseFontFileCC(tokens, fileName, line);
    } else if (!cmd->cmp("psFile")) {
      //parsePSFile(tokens, fileName, line);
    } else if (!cmd->cmp("psPaperSize")) {
      //parsePSPaperSize(tokens, fileName, line);
    } else if (!cmd->cmp("psImageableArea")) {
      //parsePSImageableArea(tokens, fileName, line);
    } else if (!cmd->cmp("psCrop")) {
      parseYesNo("psCrop", &psCrop, tokens, fileName, line);
    } else if (!cmd->cmp("psExpandSmaller")) {
      parseYesNo("psExpandSmaller", &GlobalParams::setPSExpandSmaller,
        tokens, fileName, line);
    } else if (!cmd->cmp("psShrinkLarger")) {
      parseYesNo("psShrinkLarger", &GlobalParams::setPSShrinkLarger, 
        tokens, fileName, line);
    } else if (!cmd->cmp("psCenter")) {
      parseYesNo("psCenter", &GlobalParams::setPSCenter, tokens, fileName, 
        line);
    } else if (!cmd->cmp("psDuplex")) {
      //parseYesNo("psDuplex", &psDuplex, tokens, fileName, line);
    } else if (!cmd->cmp("psLevel")) {
      //parsePSLevel(tokens, fileName, line);
    } else if (!cmd->cmp("psResidentFont")) {
      //parsePSResidentFont(tokens, fileName, line);
    } else if (!cmd->cmp("psResidentFont16")) {
      //parsePSResidentFont16(tokens, fileName, line);
    } else if (!cmd->cmp("psResidentFontCC")) {
      //parsePSResidentFontCC(tokens, fileName, line);
    } else if (!cmd->cmp("psEmbedType1Fonts")) {
      parseYesNo("psEmbedType1", &GlobalParams::setPSEmbedType1, 
        tokens, fileName, line);
    } else if (!cmd->cmp("psEmbedTrueTypeFonts")) {
      parseYesNo("psEmbedTrueType", &GlobalParams::setPSEmbedTrueType,
        tokens, fileName, line);
    } else if (!cmd->cmp("psEmbedCIDPostScriptFonts")) {
      parseYesNo("psEmbedCIDPostScript", 
        &GlobalParams::setPSEmbedCIDPostScript, tokens, fileName, line);
    } else if (!cmd->cmp("psEmbedCIDTrueTypeFonts")) {
      parseYesNo("psEmbedCIDTrueType", &GlobalParams::setPSEmbedCIDTrueType,
        tokens, fileName, line);
    } else if (!cmd->cmp("psFontPassthrough")) {
      parseYesNo("psFontPassthrough", &GlobalParams::setPSFontPassthrough,
        tokens, fileName, line);
    } else if (!cmd->cmp("psPreload")) {
      parseYesNo("psPreload", &GlobalParams::setPSPreload, tokens, 
        fileName, line);
    } else if (!cmd->cmp("psOPI")) {
      parseYesNo("psOPI", &GlobalParams::setPSOPI, tokens, fileName, line);
    } else if (!cmd->cmp("psASCIIHex")) {
      parseYesNo("psASCIIHex", &GlobalParams::setPSASCIIHex, tokens, 
        fileName, line);
    } else if (!cmd->cmp("psUncompressPreloadedImages")) {
      parseYesNo("psUncompressPreloadedImages", 
        &GlobalParams::setPSUncompressPreloadedImages, tokens, 
        fileName, line);
    } else if (!cmd->cmp("psRasterResolution")) {
      parseFloat("psRasterResolution", &GlobalParams::setPSRasterResolution,
        tokens, fileName, line);
    } else if (!cmd->cmp("psRasterMono")) {
      parseYesNo("psRasterMono", &GlobalParams::setPSRasterMono, 
        tokens, fileName, line);
    } else if (!cmd->cmp("psAlwaysRasterize")) {
      //parseYesNo("psAlwaysRasterize", &psAlwaysRasterize,
      //	 tokens, fileName, line);
    } else if (!cmd->cmp("textEncoding")) {
      //parseTextEncoding(tokens, fileName, line);
    } else if (!cmd->cmp("textEOL")) {
      //parseTextEOL(tokens, fileName, line);
    } else if (!cmd->cmp("textPageBreaks")) {
      parseYesNo("textPageBreaks", &GlobalParams::setTextPageBreaks,
        tokens, fileName, line);
    } else if (!cmd->cmp("textKeepTinyChars")) {
      parseYesNo("textKeepTinyChars", &GlobalParams::setTextKeepTinyChars,
        tokens, fileName, line);
    } else if (!cmd->cmp("initialZoom")) {
      parseInitialZoom(tokens, fileName, line);
    } else if (!cmd->cmp("continuousView")) {
      parseYesNo("continuousView", &continuousView, tokens, fileName, line);
    } else if (!cmd->cmp("enableT1lib")) {
      parseYesNo("enableT1lib", &enableT1lib, tokens, fileName, line);
    } else if (!cmd->cmp("enableFreeType")) {
      parseYesNo("enableFreeType", &GlobalParams::setEnableFreeType, 
        tokens, fileName, line);
    } else if (!cmd->cmp("disableFreeTypeHinting")) {
      parseYesNo("disableFreeTypeHinting", 
        &GlobalParams::setDisableFreeTypeHinting, tokens, fileName, 
        line);
    } else if (!cmd->cmp("antialias")) {
      parseYesNo("antialias", &GlobalParams::setAntialias, tokens, 
        fileName, line);
    } else if (!cmd->cmp("vectorAntialias")) {
      parseYesNo("vectorAntialias", &GlobalParams::setVectorAntialias,
        tokens, fileName, line);
    } else if (!cmd->cmp("antialiasPrinting")) {
      parseYesNo("antialiasPrinting", 
        &GlobalParams::setAntialiasPrinting, tokens, fileName, line);
    } else if (!cmd->cmp("strokeAdjust")) {
      parseYesNo("strokeAdjust", &GlobalParams::setStrokeAdjust, 
        tokens, fileName, line);
    } else if (!cmd->cmp("screenType")) {
      //parseScreenType(tokens, fileName, line);
    } else if (!cmd->cmp("screenSize")) {
      parseInteger("screenSize", &GlobalParams::setScreenSize, tokens, 
        fileName, line);
    } else if (!cmd->cmp("screenDotRadius")) {
      parseInteger("screenDotRadius", &GlobalParams::setScreenDotRadius,
        tokens, fileName, line);
    } else if (!cmd->cmp("screenGamma")) {
      parseFloat("screenGamma", &GlobalParams::setScreenGamma,
        tokens, fileName, line);
    } else if (!cmd->cmp("screenBlackThreshold")) {
      parseFloat("screenBlackThreshold", &GlobalParams::setScreenBlackThreshold,
        tokens, fileName, line);
    } else if (!cmd->cmp("screenWhiteThreshold")) {
      parseFloat("screenWhiteThreshold", &GlobalParams::setScreenWhiteThreshold,
        tokens, fileName, line);
    } else if (!cmd->cmp("minLineWidth")) {
      parseFloat("minLineWidth", &GlobalParams::setMinLineWidth,
        tokens, fileName, line);
    } else if (!cmd->cmp("drawAnnotations")) {
      //parseYesNo("drawAnnotations", &drawAnnotations,
      //	 tokens, fileName, line);
    } else if (!cmd->cmp("overprintPreview")) {
      parseYesNo("overprintPreview", &GlobalParams::setOverprintPreview,
        tokens, fileName, line);
    } else if (!cmd->cmp("launchCommand")) {
      parseCommand("launchCommand", launchCommand, tokens, fileName, line);
    } else if (!cmd->cmp("urlCommand")) {
      parseCommand("urlCommand", urlCommand, tokens, fileName, line);
    } else if (!cmd->cmp("movieCommand")) {
      parseCommand("movieCommand", movieCommand, tokens, fileName, line);
    } else if (!cmd->cmp("mapNumericCharNames")) {
      parseYesNo("mapNumericCharNames", &GlobalParams::setMapNumericCharNames,
        tokens, fileName, line);
    } else if (!cmd->cmp("mapUnknownCharNames")) {
      parseYesNo("mapUnknownCharNames", &GlobalParams::setMapUnknownCharNames,
        tokens, fileName, line);
    } else if (!cmd->cmp("bind")) {
      //parseBind(tokens, fileName, line);
    } else if (!cmd->cmp("unbind")) {
      //parseUnbind(tokens, fileName, line);
    } else if (!cmd->cmp("printCommands")) {
      parseYesNo("printCommands", &GlobalParams::setPrintCommands, 
        tokens, fileName, line);
    } else if (!cmd->cmp("errQuiet")) {
      parseYesNo("errQuiet", &GlobalParams::setErrQuiet, tokens, 
        fileName, line);
    } else {
      error(errConfig, -1, "Unknown config file command '{0:t}' ({1:t}:{2:d})",
	    cmd, fileName, line);
      if (!cmd->cmp("displayFontX") ||
	  !cmd->cmp("displayNamedCIDFontX") ||
	  !cmd->cmp("displayCIDFontX")) {
	error(errConfig, -1, "Xpdf no longer supports X fonts");
      } else if (!cmd->cmp("t1libControl") || !cmd->cmp("freetypeControl")) {
	error(errConfig, -1,
	      "The t1libControl and freetypeControl options have been replaced by the enableT1lib, enableFreeType, and antialias options");
      } else if (!cmd->cmp("fontpath") || !cmd->cmp("fontmap")) {
	error(errConfig, -1,
	      "The config file format has changed since Xpdf 0.9x");
      }
    }
  }

  deleteGooList(tokens, GooString);
}

void XPDFParams::parseInitialZoom(GooList *tokens,
                                  GooString *fileName, int line) {
  if (tokens->getLength() != 2) {
    error(errConfig, -1, "Bad 'initialZoom' config file command ({0:t}:{1:d})",
	  fileName, line);
    return;
  }
  initialZoom.reset(static_cast<GooString*>(tokens->get(1))->copy());
}

void XPDFParams::parseCommand(const char *cmdName, 
                              std::tr1::shared_ptr<GooString>& val,
                              GooList *tokens, GooString *fileName, int line) 
{
  if (tokens->getLength() != 2) 
  {
    error(errConfig, -1, "Bad '{0:s}' config file command ({1:t}:{2:d})",
	  cmdName, fileName, line);
    return;
  }

  val.reset(static_cast<GooString*>(tokens->get(1))->copy());
}

GBool XPDFParams::getContinuousView()
{
  lockGlobalParams;
  const GBool f = continuousView;
  unlockGlobalParams;
  return f;
}

void XPDFParams::setContinuousView(const GBool cont) 
{
  lockGlobalParams;
  continuousView = cont;
  unlockGlobalParams;
}


GBool XPDFParams::getPSCrop() 
{
  lockGlobalParams;
  const GBool f = psCrop;
  unlockGlobalParams;
  return f;
}

GooString *XPDFParams::getInitialZoom() 
{
  lockGlobalParams;
  GooString *const s = initialZoom->copy();
  unlockGlobalParams;
  return s;
}

GooList *XPDFParams::getKeyBinding(int code, int mods, int context)
{
  KeyBinding *binding;
  GooList *cmds;
  int modMask;
  int i, j;

  lockGlobalParams;
  cmds = NULL;
  // for ASCII chars, ignore the shift modifier
  modMask = code <= 0xff ? ~xpdfKeyModShift : ~0;
  for (i = 0; i < keyBindings->getLength(); ++i) {
    binding = (KeyBinding *)keyBindings->get(i);
    if (binding->code == code &&
	(binding->mods & modMask) == (mods & modMask) &&
	(~binding->context | context) == ~0) {
      cmds = new GooList();
      for (j = 0; j < binding->cmds->getLength(); ++j) {
	cmds->append(((GooString *)binding->cmds->get(j))->copy());
      }
      break;
    }
  }
  unlockGlobalParams;
  return cmds;
}

void XPDFParams::setInitialZoom(char *s) 
{
  lockGlobalParams;
  initialZoom.reset(new GooString(s));
  unlockGlobalParams;
}

GooString *XPDFParams::getLaunchCommand()
{
  return launchCommand.get();

}

GooString *XPDFParams::getMovieCommand()
{
  return movieCommand.get();
}

GooString *XPDFParams::getURLCommand()
{
  return urlCommand.get();
}

GooString *XPDFParams::getPSFile()
{
  GooString *s;

  lockGlobalParams;
  s = psFile.get() == NULL ? NULL : psFile->copy();
  unlockGlobalParams;
  return s;
}

GBool XPDFParams::setPSPaperSize(char *size) 
{
  lockGlobalParams;
  if (!strcmp(size, "match")) {
    psPaperWidth = psPaperHeight = -1;
  } else if (!strcmp(size, "letter")) {
    psPaperWidth = 612;
    psPaperHeight = 792;
  } else if (!strcmp(size, "legal")) {
    psPaperWidth = 612;
    psPaperHeight = 1008;
  } else if (!strcmp(size, "A4")) {
    psPaperWidth = 595;
    psPaperHeight = 842;
  } else if (!strcmp(size, "A3")) {
    psPaperWidth = 842;
    psPaperHeight = 1190;
  } else {
    unlockGlobalParams;
    return gFalse;
  }
  psImageableLLX = psImageableLLY = 0;
  psImageableURX = psPaperWidth;
  psImageableURY = psPaperHeight;
  unlockGlobalParams;
  return gTrue;
}

void XPDFParams::setPSPaperWidth(const int width)
{
  lockGlobalParams;
  psPaperWidth = width;
  psImageableLLX = 0;
  psImageableURX = psPaperWidth;
  unlockGlobalParams;
}

void XPDFParams::setPSPaperHeight(const int height)
{
  lockGlobalParams;
  psPaperHeight = height;
  psImageableLLY = 0;
  psImageableURY = psPaperHeight;
  unlockGlobalParams;
}

GBool XPDFParams::setEnableT1lib(char *s) 
{
  GBool ok;

  lockGlobalParams;
  ok = parseYesNo2(s, &enableT1lib);
  unlockGlobalParams;
  return ok;
}

void XPDFParams::createDefaultKeyBindings() {
  keyBindings = new GooList();

  //----- mouse buttons
  keyBindings->append(new KeyBinding(xpdfKeyCodeMousePress1, xpdfKeyModNone,
				     xpdfKeyContextAny, "startSelection"));
  keyBindings->append(new KeyBinding(xpdfKeyCodeMouseRelease1, xpdfKeyModNone,
				     xpdfKeyContextAny, "endSelection",
				     "followLinkNoSel"));
  keyBindings->append(new KeyBinding(xpdfKeyCodeMousePress2, xpdfKeyModNone,
				     xpdfKeyContextAny, "startPan"));
  keyBindings->append(new KeyBinding(xpdfKeyCodeMouseRelease2, xpdfKeyModNone,
				     xpdfKeyContextAny, "endPan"));
  keyBindings->append(new KeyBinding(xpdfKeyCodeMousePress3, xpdfKeyModNone,
				     xpdfKeyContextAny, "postPopupMenu"));
  keyBindings->append(new KeyBinding(xpdfKeyCodeMousePress4, xpdfKeyModNone,
				     xpdfKeyContextAny,
				     "scrollUpPrevPage(16)"));
  keyBindings->append(new KeyBinding(xpdfKeyCodeMousePress5, xpdfKeyModNone,
				     xpdfKeyContextAny,
				     "scrollDownNextPage(16)"));
  keyBindings->append(new KeyBinding(xpdfKeyCodeMousePress6, xpdfKeyModNone,
				     xpdfKeyContextAny, "scrollLeft(16)"));
  keyBindings->append(new KeyBinding(xpdfKeyCodeMousePress7, xpdfKeyModNone,
				     xpdfKeyContextAny, "scrollRight(16)"));

  //----- keys
  keyBindings->append(new KeyBinding(xpdfKeyCodeHome, xpdfKeyModCtrl,
				     xpdfKeyContextAny, "gotoPage(1)"));
  keyBindings->append(new KeyBinding(xpdfKeyCodeHome, xpdfKeyModNone,
				     xpdfKeyContextAny, "scrollToTopLeft"));
  keyBindings->append(new KeyBinding(xpdfKeyCodeEnd, xpdfKeyModCtrl,
				     xpdfKeyContextAny, "gotoLastPage"));
  keyBindings->append(new KeyBinding(xpdfKeyCodeEnd, xpdfKeyModNone,
				     xpdfKeyContextAny,
				     "scrollToBottomRight"));
  keyBindings->append(new KeyBinding(xpdfKeyCodePgUp, xpdfKeyModNone,
				     xpdfKeyContextAny, "pageUp"));
  keyBindings->append(new KeyBinding(xpdfKeyCodeBackspace, xpdfKeyModNone,
				     xpdfKeyContextAny, "pageUp"));
  keyBindings->append(new KeyBinding(xpdfKeyCodeDelete, xpdfKeyModNone,
				     xpdfKeyContextAny, "pageUp"));
  keyBindings->append(new KeyBinding(xpdfKeyCodePgDn, xpdfKeyModNone,
				     xpdfKeyContextAny, "pageDown"));
  keyBindings->append(new KeyBinding(' ', xpdfKeyModNone,
				     xpdfKeyContextAny, "pageDown"));
  keyBindings->append(new KeyBinding(xpdfKeyCodeLeft, xpdfKeyModNone,
				     xpdfKeyContextAny, "scrollLeft(16)"));
  keyBindings->append(new KeyBinding(xpdfKeyCodeRight, xpdfKeyModNone,
				     xpdfKeyContextAny, "scrollRight(16)"));
  keyBindings->append(new KeyBinding(xpdfKeyCodeUp, xpdfKeyModNone,
				     xpdfKeyContextAny, "scrollUp(16)"));
  keyBindings->append(new KeyBinding(xpdfKeyCodeDown, xpdfKeyModNone,
				     xpdfKeyContextAny, "scrollDown(16)"));
  keyBindings->append(new KeyBinding('o', xpdfKeyModNone,
				     xpdfKeyContextAny, "open"));
  keyBindings->append(new KeyBinding('O', xpdfKeyModNone,
				     xpdfKeyContextAny, "open"));
  keyBindings->append(new KeyBinding('r', xpdfKeyModNone,
				     xpdfKeyContextAny, "reload"));
  keyBindings->append(new KeyBinding('R', xpdfKeyModNone,
				     xpdfKeyContextAny, "reload"));
  keyBindings->append(new KeyBinding('f', xpdfKeyModNone,
				     xpdfKeyContextAny, "find"));
  keyBindings->append(new KeyBinding('F', xpdfKeyModNone,
				     xpdfKeyContextAny, "find"));
  keyBindings->append(new KeyBinding('f', xpdfKeyModCtrl,
				     xpdfKeyContextAny, "find"));
  keyBindings->append(new KeyBinding('g', xpdfKeyModCtrl,
				     xpdfKeyContextAny, "findNext"));
  keyBindings->append(new KeyBinding('p', xpdfKeyModCtrl,
				     xpdfKeyContextAny, "print"));
  keyBindings->append(new KeyBinding('n', xpdfKeyModNone,
				     xpdfKeyContextScrLockOff, "nextPage"));
  keyBindings->append(new KeyBinding('N', xpdfKeyModNone,
				     xpdfKeyContextScrLockOff, "nextPage"));
  keyBindings->append(new KeyBinding('n', xpdfKeyModNone,
				     xpdfKeyContextScrLockOn,
				     "nextPageNoScroll"));
  keyBindings->append(new KeyBinding('N', xpdfKeyModNone,
				     xpdfKeyContextScrLockOn,
				     "nextPageNoScroll"));
  keyBindings->append(new KeyBinding('p', xpdfKeyModNone,
				     xpdfKeyContextScrLockOff, "prevPage"));
  keyBindings->append(new KeyBinding('P', xpdfKeyModNone,
				     xpdfKeyContextScrLockOff, "prevPage"));
  keyBindings->append(new KeyBinding('p', xpdfKeyModNone,
				     xpdfKeyContextScrLockOn,
				     "prevPageNoScroll"));
  keyBindings->append(new KeyBinding('P', xpdfKeyModNone,
				     xpdfKeyContextScrLockOn,
				     "prevPageNoScroll"));
  keyBindings->append(new KeyBinding('v', xpdfKeyModNone,
				     xpdfKeyContextAny, "goForward"));
  keyBindings->append(new KeyBinding('b', xpdfKeyModNone,
				     xpdfKeyContextAny, "goBackward"));
  keyBindings->append(new KeyBinding('g', xpdfKeyModNone,
				     xpdfKeyContextAny, "focusToPageNum"));
  keyBindings->append(new KeyBinding('0', xpdfKeyModNone,
				     xpdfKeyContextAny, "zoomPercent(125)"));
  keyBindings->append(new KeyBinding('+', xpdfKeyModNone,
				     xpdfKeyContextAny, "zoomIn"));
  keyBindings->append(new KeyBinding('-', xpdfKeyModNone,
				     xpdfKeyContextAny, "zoomOut"));
  keyBindings->append(new KeyBinding('z', xpdfKeyModNone,
				     xpdfKeyContextAny, "zoomFitPage"));
  keyBindings->append(new KeyBinding('w', xpdfKeyModNone,
				     xpdfKeyContextAny, "zoomFitWidth"));
  keyBindings->append(new KeyBinding('f', xpdfKeyModAlt,
				     xpdfKeyContextAny,
				     "toggleFullScreenMode"));
  keyBindings->append(new KeyBinding('l', xpdfKeyModCtrl,
				     xpdfKeyContextAny, "redraw"));
  keyBindings->append(new KeyBinding('w', xpdfKeyModCtrl,
				     xpdfKeyContextAny, "closeWindow"));
  keyBindings->append(new KeyBinding('?', xpdfKeyModNone,
				     xpdfKeyContextAny, "about"));
  keyBindings->append(new KeyBinding('q', xpdfKeyModNone,
				     xpdfKeyContextAny, "quit"));
  keyBindings->append(new KeyBinding('Q', xpdfKeyModNone,
				     xpdfKeyContextAny, "quit"));
}

void XPDFParams::parseYesNo(const char *cmdName, GBool *flag,
			      GooList *tokens, GooString *fileName, int line) 
{
  if (tokens->getLength() != 2) {
    error(errConfig, -1, "Bad '{0:s}' config file command ({1:t}:{2:d})",
	  cmdName, fileName, line);
    return;
  }
  GooString *const tok = static_cast<GooString *>(tokens->get(1));
  if (!parseYesNo2(tok->getCString(), flag)) {
    error(errConfig, -1, "Bad '{0:s}' config file command ({1:t}:{2:d})",
	  cmdName, fileName, line);
  }
}

void XPDFParams::parseYesNo(const char *cmdName, 
                            void (GlobalParams::*setter)(bool),
                            GooList *tokens, GooString *fileName, int line)
{
  GBool flag;
  parseYesNo(cmdName, &flag, tokens, fileName, line);
  (globalParams->*setter)(flag);
}

void XPDFParams::parseYesNo(const char *cmdName, 
                            GBool (GlobalParams::*setter)(char*),
                            GooList *tokens, GooString *fileName, int line)
{
  char yes[] = "yes";
  char no[] = "no";
  GBool flag;
  parseYesNo(cmdName, &flag, tokens, fileName, line);
  const GBool success = (globalParams->*setter)(flag ? yes : no);
  assert(success);
}


GBool XPDFParams::parseYesNo2(char *token, GBool *flag) 
{
  if (!strcmp(token, "yes")) {
    *flag = gTrue;
  } else if (!strcmp(token, "no")) {
    *flag = gFalse;
  } else {
    return gFalse;
  }
  return gTrue;
}

void XPDFParams::parseFloat(const char *cmdName, double *val,
  GooList *tokens, GooString *fileName, int line) 
{
  GooString *tok;
  int i;

  if (tokens->getLength() != 2) {
    error(errConfig, -1, "Bad '{0:s}' config file command ({1:t}:{2:d})",
	  cmdName, fileName, line);
    return;
  }
  tok = (GooString *)tokens->get(1);
  if (tok->getLength() == 0) {
    error(errConfig, -1, "Bad '{0:s}' config file command ({1:t}:{2:d})",
	  cmdName, fileName, line);
    return;
  }
  if (tok->getChar(0) == '-') {
    i = 1;
  } else {
    i = 0;
  }
  for (; i < tok->getLength(); ++i) {
    if (!((tok->getChar(i) >= '0' && tok->getChar(i) <= '9') ||
	  tok->getChar(i) == '.')) {
      error(errConfig, -1, "Bad '{0:s}' config file command ({1:t}:{2:d})",
	    cmdName, fileName, line);
      return;
    }
  }
  *val = atof(tok->getCString());
}

void XPDFParams::parseFloat(const char *cmdName, 
  void (GlobalParams::*setter)(double), GooList *tokens, 
  GooString *fileName, int line) 
{
  double d;
  parseFloat(cmdName, &d, tokens, fileName, line);
  (globalParams->*setter)(d);
}

void XPDFParams::parseInteger(const char *cmdName, int *val,
  GooList *tokens, GooString *fileName, int line)
{
  if (tokens->getLength() != 2) 
  {
    error(errConfig, -1, "Bad '{0:s}' config file command ({1:t}:{2:d})",
	  cmdName, fileName, line);
    return;
  }

  GooString *tok = (GooString *)tokens->get(1);
  if (tok->getLength() == 0) 
  {
    error(errConfig, -1, "Bad '{0:s}' config file command ({1:t}:{2:d})",
	  cmdName, fileName, line);
    return;
  }

  int i = (tok->getChar(0) == '-') ? 1 : 0;
  for (; i < tok->getLength(); ++i) 
  {
    if (tok->getChar(i) < '0' || tok->getChar(i) > '9')
    {
      error(errConfig, -1, "Bad '{0:s}' config file command ({1:t}:{2:d})",
	    cmdName, fileName, line);
      return;
    }
  }
  *val = atoi(tok->getCString());
}

void XPDFParams::parseInteger(const char *cmdName, 
  void (GlobalParams::*setter)(int), GooList *tokens,
  GooString *fileName, int line)
{
  int value;
  parseInteger(cmdName, &value, tokens, fileName, line);
  (globalParams->*setter)(value);
}

XPDFParams::~XPDFParams()
{
  deleteGooList(keyBindings, KeyBinding);
}
