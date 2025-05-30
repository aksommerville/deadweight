#include "hero_internal.h"

/* Begin item.
 */
 
void hero_item_begin(struct sprite *sprite) {
  int equipped=store_get(NS_fld_equipped);
  fprintf(stderr,"%s equipped=%d\n",__func__,equipped);
}

/* End item.
 */
 
void hero_item_end(struct sprite *sprite) {
  fprintf(stderr,"%s\n",__func__);
}

/* Continue item use.
 */
 
void hero_item_update(struct sprite *sprite,double elapsed) {
}
