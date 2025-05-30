#ifndef GAME_H
#define GAME_H

#define FBW 256
#define FBH 224

#define MAP_LIMIT 40 /* Should match the count of map resources; more is ok. */
#define SPRDEF_LIMIT 40 /* '' sprite */
#define STORE_SIZE 64 /* bytes */
#define LISTENER_LIMIT 32
#define POI_LIMIT 32

// World size in maps. We index maps by ID and also by absolute position; they're in a single plane.
#define WORLDW 12
#define WORLDH 12

#define TRANSITION_NONE 0
#define TRANSITION_LEFT 1
#define TRANSITION_RIGHT 2
#define TRANSITION_UP 3
#define TRANSITION_DOWN 4
#define TRANSITION_RESET 5
#define TRANSITION_TELEPORT 6

#define TRANSITION_TIME 0.333

#define DIR_N 0x40
#define DIR_W 0x10
#define DIR_E 0x08
#define DIR_S 0x02

#include "egg/egg.h"
#include "opt/stdlib/egg-stdlib.h"
#include "opt/graf/graf.h"
#include "opt/text/text.h"
#include "opt/rom/rom.h"
#include "egg_rom_toc.h"
#include "shared_symbols.h"
#include "game/map.h"
#include "game/modal/modal.h"
#include "game/session/session.h"
#include "game/sprite/sprite.h"

extern struct g {

// Globals, largely immutable after init:
  void *rom;
  int romc;
  struct graf graf;
  struct font *font; // TODO Are we going to use this? Variable-width text is not very nessy.
  int texid_tilefont;
  int texid_tiles;
  int texid_sprites;
  uint8_t physics[256]; // tilesheet:tiles
  struct modal *modalv[MODAL_LIMIT];
  int modalc;
  struct map mapv[MAP_LIMIT]; // Some mutable content.
  int mapc;
  struct map *maps_by_position[WORLDW*WORLDH]; // Sparse, points into (mapv). Index is (latitute*WORLDW+longitude).
  struct sprdef sprdefv[SPRDEF_LIMIT];
  int sprdefc;
  int input;
  int pvinput;
  int input_blackout; // These bits must go false before core will report them true again.
  
// Session state:
  struct map *map; // WEAK, owned by (g.mapv).
  struct sprite **spritev;
  int spritec,spritea;
  int sprites_sort_dir;
  struct sprite *hero,*princess; // WEAK, OPTIONAL
  uint8_t store[STORE_SIZE];
  struct listener listenerv[LISTENER_LIMIT];
  int listenerc;
  int listenerid_next;
  struct poi poiv[POI_LIMIT];
  int poic;

// Room state:
  int transition;
  double transition_clock;
  int transition_texid;
} g;

extern const uint32_t nes_colors[55];
void dw_draw_string(int x,int y,const char *src,int srcc,int colorp);
void dw_draw_string_res(int x,int y,int xalign,int yalign,int rid,int ix,int colorp);

#endif
