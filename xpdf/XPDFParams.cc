#include "XPDFParams.h"
#include <gtypes.h>
#include <GooMutex.h>
#include <GooList.h>
#include <GooString.h>
#include <GlobalParams.h>
#include <Error.h>
#include "KeyBinding.h"
#include "config.h"
#include <cstring>

#if HAVE_PAPER_H
#include <paper.h>
#endif

#  define lockGlobalParams            gLockMutex(&mutex)
#  define unlockGlobalParams          gUnlockMutex(&mutex)

XPDFParams *xpdfParams = NULL;

XPDFParams::XPDFParams()
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

XPDFParams::~XPDFParams()
{
  deleteGooList(keyBindings, KeyBinding);
}
