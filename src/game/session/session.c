#include "game/game.h"

/* Receive tilesheet, there should only be one.
 */
 
static void load_tilesheet(const void *src,int srcc) {
  struct rom_tilesheet_reader reader;
  struct rom_tilesheet_entry entry;
  if (rom_tilesheet_reader_init(&reader,src,srcc)<0) return;
  while (rom_tilesheet_reader_next(&entry,&reader)>0) {
    if (entry.tableid==NS_tilesheet_physics) {
      memcpy(g.physics+entry.tileid,entry.v,entry.c);
    }
  }
}

/* Receive map resource.
 */
 
static void load_map(int rid,const void *src,int srcc) {
  struct rom_map rmap;
  if (rom_map_decode(&rmap,src,srcc)<0) return;
  if (g.mapc>=MAP_LIMIT) {
    fprintf(stderr,"Too many maps.\n");
    return;
  }
  if ((rmap.w!=NS_sys_mapw)||(rmap.h!=NS_sys_maph)) {
    fprintf(stderr,"map:%d incorrect size %dx%d, expected %dx%d\n",rid,rmap.w,rmap.h,NS_sys_mapw,NS_sys_maph);
    return;
  }
  struct map *map=g.mapv+g.mapc++;
  map->rid=rid;
  map->rocellv=rmap.v;
  map->cmdv=rmap.cmdv;
  map->cmdc=rmap.cmdc;
}

/* Receive sprite resource.
 */
 
static void load_sprite(int rid,const void *src,int srcc) {
  struct rom_sprite rspr;
  if (rom_sprite_decode(&rspr,src,srcc)<0) return;
  if (g.sprdefc>=SPRDEF_LIMIT) {
    fprintf(stderr,"Too many sprite resources.\n");
    return;
  }
  const struct sprite_type *type=0;
  struct rom_command_reader reader={.v=rspr.cmdv,.c=rspr.cmdc};
  struct rom_command cmd;
  while (rom_command_reader_next(&cmd,&reader)>0) {
    switch (cmd.opcode) {
      case CMD_sprite_sprtype: type=sprite_type_by_id((cmd.argv[0]<<8)|cmd.argv[1]); break;
    }
  }
  if (!type) {
    fprintf(stderr,"sprite:%d: Type unknown.\n",rid);
    return;
  }
  struct sprdef *sprdef=g.sprdefv+g.sprdefc++;
  sprdef->rid=rid;
  sprdef->cmdv=rspr.cmdv;
  sprdef->cmdc=rspr.cmdc;
  sprdef->type=type;
}

/* Init.
 */
 
int session_init() {
  
  /* Load all resources.
   */
  struct rom_reader reader;
  if (rom_reader_init(&reader,g.rom,g.romc)<0) return -1;
  struct rom_res *res;
  while (res=rom_reader_next(&reader)) {
    switch (res->tid) {
      case EGG_TID_tilesheet: if (res->rid==RID_tilesheet_tiles) load_tilesheet(res->v,res->c); break;
      case EGG_TID_map: load_map(res->rid,res->v,res->c); break;
      case EGG_TID_sprite: load_sprite(res->rid,res->v,res->c); break;
    }
  }
  
  return 0;
}

/* Reset.
 */

int session_reset() {

  // Restore all map content to their defaults.
  struct map *map=g.mapv;
  int i=g.mapc;
  for (;i-->0;map++) memcpy(map->cellv,map->rocellv,sizeof(map->cellv));

  //TODO Wipe store.
  
  if (enter_map(RID_map_home,TRANSITION_NONE)<0) {
    fprintf(stderr,"Loading map:%d failed.\n",RID_map_home);
    return -1;
  }
  
  return 0;
}

/* Enter map.
 */
 
int enter_map(int rid,int transition) {
  struct map *map=map_by_id(rid);
  if (!map) return -1;
  g.map=map;
  
  sprites_kill_all();
  //TODO How are we managing the hero and princess sprites? They either persist across maps or get recreated.
  
  struct rom_command_reader reader={.v=g.map->cmdv,.c=g.map->cmdc};
  struct rom_command cmd;
  while (rom_command_reader_next(&cmd,&reader)>0) {
    switch (cmd.opcode) {
      case CMD_map_sprite: {
          uint8_t x=cmd.argv[0];
          uint8_t y=cmd.argv[1];
          uint16_t rid=(cmd.argv[2]<<8)|cmd.argv[3];
          uint32_t arg=(cmd.argv[4]<<24)|(cmd.argv[5]<<16)|(cmd.argv[6]<<8)|cmd.argv[7];
          sprite_spawn(x+0.5,y+0.5,rid,0,arg);
        } break;
    }
  }
  
  //TODO Prepare transition.
  
  return 0;
}
