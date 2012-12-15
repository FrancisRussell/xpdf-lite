#include "KeyBinding.h"
#include <GooString.h>
#include <GooList.h>

KeyBinding::KeyBinding(int codeA, int modsA, int contextA, const char *cmd0) {
  code = codeA;
  mods = modsA;
  context = contextA;
  cmds = new GooList();
  cmds->append(new GooString(cmd0));
}

KeyBinding::KeyBinding(int codeA, int modsA, int contextA,
		       const char *cmd0, const char *cmd1) {
  code = codeA;
  mods = modsA;
  context = contextA;
  cmds = new GooList();
  cmds->append(new GooString(cmd0));
  cmds->append(new GooString(cmd1));
}

KeyBinding::KeyBinding(int codeA, int modsA, int contextA, GooList *cmdsA) {
  code = codeA;
  mods = modsA;
  context = contextA;
  cmds = cmdsA;
}

KeyBinding::~KeyBinding() {
  deleteGooList(cmds, GooString);
}
