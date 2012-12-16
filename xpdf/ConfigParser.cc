#include "ConfigParser.h"
#include "config.h"
#include "XPDFParams.h"
#include "KeyBinding.h"
#include <Error.h>
#include <GooString.h>
#include <GooList.h>
#include <gfile.h>
#include <GlobalParams.h>
#include <cctype>
#include <cassert>
#include <cstring>
#include <cstdio>

ConfigParser::ConfigParser(GlobalParams *_pParams, XPDFParams *_xParams) :
  pParams(_pParams), xParams(_xParams)
{
}

void ConfigParser::loadConfig(const char* configPath)
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

void ConfigParser::parseFile(GooString *fileName, FILE *f) 
{
  char buf[512];
  int line = 1;

  while (getLine(buf, sizeof(buf) - 1, f)) 
  {
    parseLine(buf, fileName, line);
    ++line;
  }
}

void ConfigParser::parseLine(char *buf, GooString *fileName, int line) 
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
      parseFontFile(tokens, fileName, line);
    } else if (!cmd->cmp("fontDir")) {
      //parseFontDir(tokens, fileName, line);
    } else if (!cmd->cmp("fontFileCC")) {
      //parseFontFileCC(tokens, fileName, line);
    } else if (!cmd->cmp("psFile")) {
      parsePSFile(tokens, fileName, line);
    } else if (!cmd->cmp("psPaperSize")) {
      //parsePSPaperSize(tokens, fileName, line);
    } else if (!cmd->cmp("psImageableArea")) {
      //parsePSImageableArea(tokens, fileName, line);
    } else if (!cmd->cmp("psCrop")) {
      parseYesNo("psCrop", &XPDFParams::setPSCrop, tokens, fileName, line);
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
      parsePSLevel(tokens, fileName, line);
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
      parseTextEncoding(tokens, fileName, line);
    } else if (!cmd->cmp("textEOL")) {
      parseTextEOL(tokens, fileName, line);
    } else if (!cmd->cmp("textPageBreaks")) {
      parseYesNo("textPageBreaks", &GlobalParams::setTextPageBreaks,
        tokens, fileName, line);
    } else if (!cmd->cmp("textKeepTinyChars")) {
      parseYesNo("textKeepTinyChars", &GlobalParams::setTextKeepTinyChars,
        tokens, fileName, line);
    } else if (!cmd->cmp("initialZoom")) {
      parseInitialZoom(tokens, fileName, line);
    } else if (!cmd->cmp("continuousView")) {
      parseYesNo("continuousView", &XPDFParams::setContinuousView, 
        tokens, fileName, line);
    } else if (!cmd->cmp("enableT1lib")) {
      parseYesNo("enableT1lib", &XPDFParams::setEnableT1lib, tokens, 
        fileName, line);
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
      parseScreenType(tokens, fileName, line);
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
      parseCommand("launchCommand", &XPDFParams::setLaunchCommand, 
        tokens, fileName, line);
    } else if (!cmd->cmp("urlCommand")) {
      parseCommand("urlCommand", &XPDFParams::setURLCommand, tokens, 
        fileName, line);
    } else if (!cmd->cmp("movieCommand")) {
      parseCommand("movieCommand", &XPDFParams::setMovieCommand, 
        tokens, fileName, line);
    } else if (!cmd->cmp("mapNumericCharNames")) {
      parseYesNo("mapNumericCharNames", &GlobalParams::setMapNumericCharNames,
        tokens, fileName, line);
    } else if (!cmd->cmp("mapUnknownCharNames")) {
      parseYesNo("mapUnknownCharNames", &GlobalParams::setMapUnknownCharNames,
        tokens, fileName, line);
    } else if (!cmd->cmp("bind")) {
      parseBind(tokens, fileName, line);
    } else if (!cmd->cmp("unbind")) {
      parseUnbind(tokens, fileName, line);
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



void ConfigParser::parseInitialZoom(GooList *tokens,
                                  GooString *fileName, int line) {
  if (tokens->getLength() != 2) {
    error(errConfig, -1, "Bad 'initialZoom' config file command ({0:t}:{1:d})",
	  fileName, line);
    return;
  }
  xParams->setInitialZoom(static_cast<GooString*>(tokens->get(1))->getCString());
}

void ConfigParser::parseCommand(const char *cmdName, 
                              void (XPDFParams::*setter)(const char*),
                              GooList *tokens, GooString *fileName, int line) 
{
  if (tokens->getLength() != 2) 
  {
    error(errConfig, -1, "Bad '{0:s}' config file command ({1:t}:{2:d})",
	  cmdName, fileName, line);
    return;
  }

  (xParams->*setter)(static_cast<GooString*>(tokens->get(1))->getCString());
}

void ConfigParser::parseYesNo(const char *cmdName, GBool *flag,
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

void ConfigParser::parseYesNo(const char *cmdName, 
                            void (GlobalParams::*setter)(bool),
                            GooList *tokens, GooString *fileName, int line)
{
  GBool flag;
  parseYesNo(cmdName, &flag, tokens, fileName, line);
  (pParams->*setter)(flag);
}

void ConfigParser::parseYesNo(const char *cmdName, 
                            void (XPDFParams::*setter)(bool),
                            GooList *tokens, GooString *fileName, int line)
{
  GBool flag;
  parseYesNo(cmdName, &flag, tokens, fileName, line);
  (xParams->*setter)(flag);
}


void ConfigParser::parseYesNo(const char *cmdName, 
                            GBool (GlobalParams::*setter)(char*),
                            GooList *tokens, GooString *fileName, int line)
{
  char yes[] = "yes";
  char no[] = "no";
  GBool flag;
  parseYesNo(cmdName, &flag, tokens, fileName, line);
  const GBool success = (pParams->*setter)(flag ? yes : no);
  assert(success);
}

void ConfigParser::parseYesNo(const char *cmdName, 
                            GBool (XPDFParams::*setter)(char*),
                            GooList *tokens, GooString *fileName, int line)
{
  char yes[] = "yes";
  char no[] = "no";
  GBool flag;
  parseYesNo(cmdName, &flag, tokens, fileName, line);
  const GBool success = (xParams->*setter)(flag ? yes : no);
  assert(success);
}



GBool ConfigParser::parseYesNo2(char *token, GBool *flag) 
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

void ConfigParser::parseFloat(const char *cmdName, double *val,
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

void ConfigParser::parseFloat(const char *cmdName, 
  void (GlobalParams::*setter)(double), GooList *tokens, 
  GooString *fileName, int line) 
{
  double d;
  parseFloat(cmdName, &d, tokens, fileName, line);
  (pParams->*setter)(d);
}

void ConfigParser::parseInteger(const char *cmdName, int *val,
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

void ConfigParser::parseInteger(const char *cmdName, 
  void (GlobalParams::*setter)(int), GooList *tokens,
  GooString *fileName, int line)
{
  int value;
  parseInteger(cmdName, &value, tokens, fileName, line);
  (pParams->*setter)(value);
}

void ConfigParser::parseBind(GooList *tokens, GooString *fileName, int line) 
{
  int code, mods, context;

  if (tokens->getLength() < 4) 
  {
    error(errConfig, -1, "Bad 'bind' config file command ({0:t}:{1:d})",
	  fileName, line);
    return;
  }

  if (!parseKey((GooString *)tokens->get(1), (GooString *)tokens->get(2),
		&code, &mods, &context,
		"bind", tokens, fileName, line)) 
  {
    return;
  }

  GooList *cmds = new GooList();
  for (int i = 3; i < tokens->getLength(); ++i) 
    cmds->append(((GooString *)tokens->get(i))->copy());

  xParams->addKeyBinding(code, mods, context, cmds);
}

void ConfigParser::parseUnbind(GooList *tokens, GooString *fileName, int line) 
{
  KeyBinding *binding;
  int code, mods, context, i;

  if (tokens->getLength() != 3) {
    error(errConfig, -1, "Bad 'unbind' config file command ({0:t}:{1:d})",
	  fileName, line);
    return;
  }
  if (!parseKey((GooString *)tokens->get(1), (GooString *)tokens->get(2),
		&code, &mods, &context,
		"unbind", tokens, fileName, line)) {
    return;
  }

  xParams->deleteKeyBinding(code, mods, context);
}

GBool ConfigParser::parseKey(GooString *modKeyStr, GooString *contextStr,
			     int *code, int *mods, int *context,
			     const char *cmdName,
			     GooList *tokens, GooString *fileName, int line) 
{
  char *p0;
  int btn;

  *mods = xpdfKeyModNone;
  p0 = modKeyStr->getCString();
  while (1) {
    if (!strncmp(p0, "shift-", 6)) {
      *mods |= xpdfKeyModShift;
      p0 += 6;
    } else if (!strncmp(p0, "ctrl-", 5)) {
      *mods |= xpdfKeyModCtrl;
      p0 += 5;
    } else if (!strncmp(p0, "alt-", 4)) {
      *mods |= xpdfKeyModAlt;
      p0 += 4;
    } else {
      break;
    }
  }

  if (!strcmp(p0, "space")) {
    *code = ' ';
  } else if (!strcmp(p0, "tab")) {
    *code = xpdfKeyCodeTab;
  } else if (!strcmp(p0, "return")) {
    *code = xpdfKeyCodeReturn;
  } else if (!strcmp(p0, "enter")) {
    *code = xpdfKeyCodeEnter;
  } else if (!strcmp(p0, "backspace")) {
    *code = xpdfKeyCodeBackspace;
  } else if (!strcmp(p0, "insert")) {
    *code = xpdfKeyCodeInsert;
  } else if (!strcmp(p0, "delete")) {
    *code = xpdfKeyCodeDelete;
  } else if (!strcmp(p0, "home")) {
    *code = xpdfKeyCodeHome;
  } else if (!strcmp(p0, "end")) {
    *code = xpdfKeyCodeEnd;
  } else if (!strcmp(p0, "pgup")) {
    *code = xpdfKeyCodePgUp;
  } else if (!strcmp(p0, "pgdn")) {
    *code = xpdfKeyCodePgDn;
  } else if (!strcmp(p0, "left")) {
    *code = xpdfKeyCodeLeft;
  } else if (!strcmp(p0, "right")) {
    *code = xpdfKeyCodeRight;
  } else if (!strcmp(p0, "up")) {
    *code = xpdfKeyCodeUp;
  } else if (!strcmp(p0, "down")) {
    *code = xpdfKeyCodeDown;
  } else if (p0[0] == 'f' && p0[1] >= '1' && p0[1] <= '9' && !p0[2]) {
    *code = xpdfKeyCodeF1 + (p0[1] - '1');
  } else if (p0[0] == 'f' &&
	     ((p0[1] >= '1' && p0[1] <= '2' && p0[2] >= '0' && p0[2] <= '9') ||
	      (p0[1] == '3' && p0[2] >= '0' && p0[2] <= '5')) &&
	     !p0[3]) {
    *code = xpdfKeyCodeF1 + 10 * (p0[1] - '0') + (p0[2] - '0') - 1;
  } else if (!strncmp(p0, "mousePress", 10) &&
	     p0[10] >= '0' && p0[10] <= '9' &&
	     (!p0[11] || (p0[11] >= '0' && p0[11] <= '9' && !p0[12])) &&
	     (btn = atoi(p0 + 10)) >= 1 && btn <= 32) {
    *code = xpdfKeyCodeMousePress1 + btn - 1;
  } else if (!strncmp(p0, "mouseRelease", 12) &&
	     p0[12] >= '0' && p0[12] <= '9' &&
	     (!p0[13] || (p0[13] >= '0' && p0[13] <= '9' && !p0[14])) &&
	     (btn = atoi(p0 + 12)) >= 1 && btn <= 32) {
    *code = xpdfKeyCodeMouseRelease1 + btn - 1;
  } else if (*p0 >= 0x20 && *p0 <= 0x7e && !p0[1]) {
    *code = (int)*p0;
  } else {
    error(errConfig, -1,
	  "Bad key/modifier in '{0:s}' config file command ({1:t}:{2:d})",
	  cmdName, fileName, line);
    return gFalse;
  }

  p0 = contextStr->getCString();
  if (!strcmp(p0, "any")) {
    *context = xpdfKeyContextAny;
  } else {
    *context = xpdfKeyContextAny;
    while (1) {
      if (!strncmp(p0, "fullScreen", 10)) {
	*context |= xpdfKeyContextFullScreen;
	p0 += 10;
      } else if (!strncmp(p0, "window", 6)) {
	*context |= xpdfKeyContextWindow;
	p0 += 6;
      } else if (!strncmp(p0, "continuous", 10)) {
	*context |= xpdfKeyContextContinuous;
	p0 += 10;
      } else if (!strncmp(p0, "singlePage", 10)) {
	*context |= xpdfKeyContextSinglePage;
	p0 += 10;
      } else if (!strncmp(p0, "overLink", 8)) {
	*context |= xpdfKeyContextOverLink;
	p0 += 8;
      } else if (!strncmp(p0, "offLink", 7)) {
	*context |= xpdfKeyContextOffLink;
	p0 += 7;
      } else if (!strncmp(p0, "outline", 7)) {
	*context |= xpdfKeyContextOutline;
	p0 += 7;
      } else if (!strncmp(p0, "mainWin", 7)) {
	*context |= xpdfKeyContextMainWin;
	p0 += 7;
      } else if (!strncmp(p0, "scrLockOn", 9)) {
	*context |= xpdfKeyContextScrLockOn;
	p0 += 9;
      } else if (!strncmp(p0, "scrLockOff", 10)) {
	*context |= xpdfKeyContextScrLockOff;
	p0 += 10;
      } else {
	error(errConfig, -1,
	      "Bad context in '{0:s}' config file command ({1:t}:{2:d})",
	      cmdName, fileName, line);
	return gFalse;
      }
      if (!*p0) {
	break;
      }
      if (*p0 != ',') {
	error(errConfig, -1,
	      "Bad context in '{0:s}' config file command ({1:t}:{2:d})",
	      cmdName, fileName, line);
	return gFalse;
      }
      ++p0;
    }
  }

  return gTrue;
}

void ConfigParser::parsePSLevel(GooList *tokens, GooString *fileName, int line) 
{
  if (tokens->getLength() != 2) {
    error(errConfig, -1, "Bad 'psLevel' config file command ({0:t}:{1:d})",
	  fileName, line);
    return;
  }
  GooString *tok = (GooString *)tokens->get(1);
  if (!tok->cmp("level1")) {
    pParams->setPSLevel(psLevel1);
  } else if (!tok->cmp("level1sep")) {
    pParams->setPSLevel(psLevel1Sep);
  } else if (!tok->cmp("level2")) {
    pParams->setPSLevel(psLevel2);
  } else if (!tok->cmp("level2sep")) {
    pParams->setPSLevel(psLevel2Sep);
  } else if (!tok->cmp("level3")) {
    pParams->setPSLevel(psLevel3);
  } else if (!tok->cmp("level3Sep")) {
    pParams->setPSLevel(psLevel3Sep);
  } else {
    error(errConfig, -1, "Bad 'psLevel' config file command ({0:t}:{1:d})",
	  fileName, line);
  }
}

void ConfigParser::parsePSFile(GooList *tokens, GooString *fileName, int line) 
{
  if (tokens->getLength() != 2) {
    error(errConfig, -1, "Bad 'psFile' config file command ({0:t}:{1:d})",
	  fileName, line);
    return;
  }

  xParams->setPSFile(static_cast<GooString*>(tokens->get(1))->getCString());
}

void ConfigParser::parseScreenType(GooList *tokens, GooString *fileName,
  int line) 
{
  GooString *tok;

  if (tokens->getLength() != 2) 
  {
    error(errConfig, -1, "Bad 'screenType' config file command ({0:t}:{1:d})",
	  fileName, line);
    return;
  }

  tok = (GooString *)tokens->get(1);
  if (!tok->cmp("dispersed")) 
    pParams->setScreenType(screenDispersed);
  else if (!tok->cmp("clustered")) 
    pParams->setScreenType(screenClustered);
  else if (!tok->cmp("stochasticClustered")) 
    pParams->setScreenType(screenStochasticClustered);
  else 
    error(errConfig, -1, "Bad 'screenType' config file command ({0:t}:{1:d})",
	  fileName, line);
}

void ConfigParser::parseTextEncoding(GooList *tokens, GooString *fileName, 
       int line) 
{
  if (tokens->getLength() != 2) {
    error(errConfig, -1,
	  "Bad 'textEncoding' config file command ({0:s}:{1:d})",
	  fileName, line);
    return;
  }
  pParams->setTextEncoding(static_cast<GooString*>(tokens->get(1))->getCString());
}

void ConfigParser::parseTextEOL(GooList *tokens, GooString *fileName, 
  int line) 
{ 
  if (tokens->getLength() != 2) {
    error(errConfig, -1, "Bad 'textEOL' config file command ({0:t}:{1:d})",
	  fileName, line);
    return;
  }
  GooString *tok = (GooString *)tokens->get(1);
  if (!pParams->setTextEOL(tok->getCString())) 
  {
    error(errConfig, -1, "Bad 'textEOL' config file command ({0:t}:{1:d})",
	  fileName, line);
  }
}

void ConfigParser::parseFontFile(GooList *tokens, GooString *fileName, int line) 
{
  if (tokens->getLength() != 3)
  {
    error(errConfig, -1, "Bad 'fontFile' config file command ({0:t}:{1:d})",
	  fileName, line);
    return;
  }
  pParams->addFontFile(((GooString *)tokens->get(1))->copy(),
		            ((GooString *)tokens->get(2))->copy());
}



