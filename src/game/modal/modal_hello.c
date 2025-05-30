#include "game/game.h"

struct modal_hello {
  struct modal hdr;
};

#define MODAL ((struct modal_hello*)modal)

/* Cleanup.
 */
 
static void _hello_del(struct modal *modal) {
}

/* Digested input.
 */
 
static void _hello_move(struct modal *modal,int dx,int dy) {
  fprintf(stderr,"%s %+d,%+d\n",__func__,dx,dy);
}

static void _hello_activate(struct modal *modal) {
  fprintf(stderr,"%s\n",__func__);
  if (session_reset()<0) return;
  modal->defunct=1;
  modal_new_play();
}

static void _hello_cancel(struct modal *modal) {
  fprintf(stderr,"%s\n",__func__);
}

/* Input state change.
 */
 
static void _hello_input(struct modal *modal) {
  fprintf(stderr,"%s 0x%04x\n",__func__,g.input);
  if ((g.input&EGG_BTN_LEFT)&&!(g.pvinput&EGG_BTN_LEFT)) _hello_move(modal,-1,0);
  if ((g.input&EGG_BTN_RIGHT)&&!(g.pvinput&EGG_BTN_RIGHT)) _hello_move(modal,1,0);
  if ((g.input&EGG_BTN_UP)&&!(g.pvinput&EGG_BTN_UP)) _hello_move(modal,0,-1);
  if ((g.input&EGG_BTN_DOWN)&&!(g.pvinput&EGG_BTN_DOWN)) _hello_move(modal,0,1);
  if ((g.input&EGG_BTN_SOUTH)&&!(g.pvinput&EGG_BTN_SOUTH)) {
    g.input_blackout|=EGG_BTN_SOUTH;
    _hello_activate(modal);
  }
  if ((g.input&EGG_BTN_WEST)&&!(g.pvinput&EGG_BTN_WEST)) _hello_cancel(modal);
  if ((g.input&EGG_BTN_AUX1)&&!(g.pvinput&EGG_BTN_AUX1)) {
    g.input_blackout|=EGG_BTN_AUX1;
    _hello_activate(modal);
  }
}

/* Update.
 */
 
static void _hello_update(struct modal *modal,double elapsed) {
}

/* Render.
 */
 
static void _hello_render(struct modal *modal) {
  //TODO
  graf_draw_rect(&g.graf,0,0,FBW,FBH,0x0000ffff);
  dw_draw_string(20,20,"Rescue the princess!",-1,1);
  dw_draw_string(20,28,"Press \x14 to use or \x15 to choose.",-1,1);
  dw_draw_string(20,36,"\x10 \x11 \x12 \x13",-1,18);
}

/* Init.
 */
 
static int _hello_init(struct modal *modal) {
  modal->name="hello";
  modal->del=_hello_del;
  modal->input=_hello_input;
  modal->update=_hello_update;
  modal->render=_hello_render;
  modal->opaque=1;
  modal->passive=0;
  return 0;
}

/* New.
 */
 
struct modal *modal_new_hello() {
  struct modal *modal=modal_new(sizeof(struct modal_hello));
  if (!modal) return 0;
  if (_hello_init(modal)<0) {
    modal->defunct=1;
    return 0;
  }
  return modal;
}
