#include "game/game.h"

struct modal_dialogue {
  struct modal hdr;
};

#define MODAL ((struct modal_dialogue*)modal)

/* Cleanup.
 */
 
static void _dialogue_del(struct modal *modal) {
}

/* Update.
 */
 
static void _dialogue_update(struct modal *modal,double elapsed) {
}

/* Render.
 */
 
static void _dialogue_render(struct modal *modal) {
  graf_draw_rect(&g.graf,0,0,FBW,FBH,0xffffffc0);
}

/* Init.
 */
 
static int _dialogue_init(struct modal *modal) {
  modal->name="dialogue";
  modal->del=_dialogue_del;
  modal->update=_dialogue_update;
  modal->render=_dialogue_render;
  modal->opaque=0;
  modal->passive=0;
  return 0;
}

/* New.
 */
 
struct modal *modal_new_dialogue() {
  struct modal *modal=modal_new(sizeof(struct modal_dialogue));
  if (!modal) return 0;
  if (_dialogue_init(modal)<0) {
    modal->defunct=1;
    return 0;
  }
  return modal;
}
