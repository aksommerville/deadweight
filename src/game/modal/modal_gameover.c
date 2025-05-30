#include "game/game.h"

struct modal_gameover {
  struct modal hdr;
};

#define MODAL ((struct modal_gameover*)modal)

/* Cleanup.
 */
 
static void _gameover_del(struct modal *modal) {
}

/* Input.
 */
 
static void _gameover_input(struct modal *modal) {
}

/* Update.
 */
 
static void _gameover_update(struct modal *modal,double elapsed) {
}

/* Render.
 */
 
static void _gameover_render(struct modal *modal) {
  graf_draw_rect(&g.graf,0,0,FBW,FBH,0xff0000ff);
}

/* Init.
 */
 
static int _gameover_init(struct modal *modal) {
  modal->name="gameover";
  modal->del=_gameover_del;
  modal->input=_gameover_input;
  modal->update=_gameover_update;
  modal->render=_gameover_render;
  modal->opaque=1;
  modal->passive=0;
  return 0;
}

/* New.
 */
 
struct modal *modal_new_gameover() {
  struct modal *modal=modal_new(sizeof(struct modal_gameover));
  if (!modal) return 0;
  if (_gameover_init(modal)<0) {
    modal->defunct=1;
    return 0;
  }
  return modal;
}
