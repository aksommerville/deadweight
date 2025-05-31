#include "game/game.h"

/* Get map resource by id.
 */
 
struct map *map_by_id(int rid) {
  int lo=0,hi=g.mapc;
  while (lo<hi) {
    int ck=(lo+hi)>>1;
    struct map *q=g.mapv+ck;
         if (rid<q->rid) hi=ck;
    else if (rid>q->rid) lo=ck+1;
    else return q;
  }
  return 0;
}

/* Receive map resource.
 */
 
int load_map(int rid,const void *src,int srcc) {
  struct rom_map rmap;
  if (rom_map_decode(&rmap,src,srcc)<0) return -1;
  if (g.mapc>=MAP_LIMIT) {
    fprintf(stderr,"Too many maps.\n");
    return -1;
  }
  if ((rmap.w!=NS_sys_mapw)||(rmap.h!=NS_sys_maph)) {
    fprintf(stderr,"map:%d incorrect size %dx%d, expected %dx%d\n",rid,rmap.w,rmap.h,NS_sys_mapw,NS_sys_maph);
    return -1;
  }
  struct map *map=g.mapv+g.mapc++;
  map->rid=rid;
  map->rocellv=rmap.v;
  map->cmdv=rmap.cmdv;
  map->cmdc=rmap.cmdc;
  
  struct rom_command_reader reader={.v=rmap.cmdv,.c=rmap.cmdc};
  struct rom_command cmd;
  while (rom_command_reader_next(&cmd,&reader)>0) {
    switch (cmd.opcode) {
      case CMD_map_location: {
          uint8_t longitude=cmd.argv[0];
          uint8_t latitude=cmd.argv[1];
          if ((longitude>=WORLDW)||(latitude>=WORLDH)) {
            fprintf(stderr,"map:%d claims position (%d,%d) in world of size (%d,%d)\n",rid,longitude,latitude,WORLDW,WORLDH);
            return -2;
          }
          int p=latitude*WORLDW+longitude;
          if (g.maps_by_position[p]) {
            fprintf(stderr,"World position (%d,%d) claimed by map:%d and map:%d\n",longitude,latitude,g.maps_by_position[p]->rid,rid);
            return -2;
          }
          g.maps_by_position[p]=map;
          map->longitude=longitude;
          map->latitude=latitude;
        } break;
    }
  }
  
  return 0;
}
