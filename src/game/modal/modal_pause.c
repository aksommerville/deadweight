#include "game/game.h"

struct modal_pause {
  struct modal hdr;
};

#define MODAL ((struct modal_pause*)modal)

/* Cleanup.
 */
 
static void _pause_del(struct modal *modal) {
}

/* Update.
 */
 
static void _pause_update(struct modal *modal,double elapsed) {
}

/* Render.
 */
 
static void _pause_render(struct modal *modal) {
  graf_draw_rect(&g.graf,0,0,FBW,FBH,0x000000c0);
}

/* Init.
 */
 
static int _pause_init(struct modal *modal) {
  modal->name="pause";
  modal->del=_pause_del;
  modal->update=_pause_update;
  modal->render=_pause_render;
  modal->opaque=0;
  modal->passive=0;
  return 0;
}

/* New.
 */
 
struct modal *modal_new_pause() {
  struct modal *modal=modal_new(sizeof(struct modal_pause));
  if (!modal) return 0;
  if (_pause_init(modal)<0) {
    modal->defunct=1;
    return 0;
  }
  return modal;
}
