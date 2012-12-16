#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include <cstdio>
#include <tr1/memory>
#include "gtypes.h"

class GlobalParams;
class XPDFParams;
class GooString;
class GooList;

class ConfigParser
{
private:
  GlobalParams *pParams;
  XPDFParams *xParams;

  // Present in poppler, but preferable as static utility methods
  static void parseFloat(const char *cmdName, double *val,
    GooList *tokens, GooString *fileName, int line);
  static void parseInteger(const char *cmdName, int *val,
    GooList *tokens, GooString *fileName, int line);

  // For parsing values passed to GlobalParams
  void parseYesNo(const char *cmdName, 
    void (GlobalParams::*setter)(bool), GooList *tokens, 
    GooString *fileName, int line);
  void parseYesNo(const char *cmdName,
    GBool (GlobalParams::*setter)(char*), GooList *tokens, 
    GooString *fileName, int line);
  void parseFloat(const char *cmdName, 
    void (GlobalParams::*setter)(double),
    GooList *tokens, GooString *fileName, int line);
  void parseInteger(const char *cmdName,
      void (GlobalParams::*setter)(int), GooList *tokens, 
      GooString *fileName, int line);

  // For parsing values passed to XPDFParams
  void parseYesNo(const char *cmdName, 
    void (XPDFParams::*setter)(bool), GooList *tokens, 
    GooString *fileName, int line);
  void parseYesNo(const char *cmdName,
    GBool (XPDFParams::*setter)(char*), GooList *tokens, 
    GooString *fileName, int line);

  void parseFile(GooString *fileName, FILE *f);
  void parseLine(char *buf, GooString *fileName, int line);
  void parsePSFile(GooList *tokens, GooString *fileName, int line);
  void parsePSPaperSize(GooList *tokens, GooString *fileName, int line);
  void parsePSImageableArea(GooList *tokens, GooString *fileName, int line);
  void parseInitialZoom(GooList *tokens, GooString *fileName, int line);
  void parseCommand(const char *cmdName, 
         void (XPDFParams::*setter)(const char*), GooList *tokens, 
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

public:
  ConfigParser(GlobalParams *pParams, XPDFParams *xParams);
  static GBool parseYesNo2(char *token, GBool *flag);
  static void parseYesNo(const char *cmdName, GBool *flag, GooList *tokens, GooString *fileName, int line);
  void loadConfig(const char *configPath);
};

#endif
