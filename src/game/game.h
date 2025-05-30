#ifndef GAME_H
#define GAME_H

#define FBW 256
#define FBH 224

#include "egg/egg.h"
#include "opt/stdlib/egg-stdlib.h"
#include "opt/graf/graf.h"
#include "opt/text/text.h"
#include "egg_rom_toc.h"
#include "shared_symbols.h"

extern struct g {
  void *rom;
  int romc;
  struct graf graf;
  struct font *font; // TODO Are we going to use this? Variable-width text is not very nessy.
} g;

#endif
