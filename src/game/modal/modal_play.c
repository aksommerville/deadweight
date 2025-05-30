#include "game/game.h"

struct modal_play {
  struct modal hdr;
};

#define MODAL ((struct modal_play*)modal)

/* Cleanup.
 */
 
static void _play_del(struct modal *modal) {
}

/* Update.
 */
 
static void _play_update(struct modal *modal,double elapsed) {
}

/* Render.
 */
 
static void _play_render(struct modal *modal) {
  graf_draw_rect(&g.graf,0,0,FBW,FBH,0x00ff00ff);
}

/* Init.
 */
 
static int _play_init(struct modal *modal) {
  modal->name="play";
  modal->del=_play_del;
  modal->update=_play_update;
  modal->render=_play_render;
  modal->opaque=1;
  modal->passive=0;
  return 0;
}

/* New.
 */
 
struct modal *modal_new_play() {
  struct modal *modal=modal_new(sizeof(struct modal_play));
  if (!modal) return 0;
  if (_play_init(modal)<0) {
    modal->defunct=1;
    return 0;
  }
  return modal;
}
