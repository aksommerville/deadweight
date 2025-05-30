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
