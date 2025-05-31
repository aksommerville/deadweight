/* map.h
 */
 
#ifndef MAP_H
#define MAP_H

struct map {
  int rid;
  uint8_t latitude,longitude; // Position in the world, ie (g.maps_by_position).
  const uint8_t *rocellv;
  uint8_t cellv[NS_sys_mapw*NS_sys_maph];
  const uint8_t *cmdv;
  int cmdc;
};

struct poi {
  uint8_t x,y;
  uint8_t opcode;
  const uint8_t *argv; // [0,1] are always (x,y)
  int argc;
};

// Session calls this once per resource at startup:
int load_map(int rid,const void *src,int srcc);

struct map *map_by_id(int rid);

#endif
