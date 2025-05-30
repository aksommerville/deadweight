/* map.h
 */
 
#ifndef MAP_H
#define MAP_H

struct map {
  int rid;
  const uint8_t *rocellv;
  uint8_t cellv[NS_sys_mapw*NS_sys_maph];
  const uint8_t *cmdv;
  int cmdc;
};

struct map *map_by_id(int rid);

#endif
