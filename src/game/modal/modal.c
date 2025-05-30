#include "game/game.h"

/* Global input state changed.
 */
 
void modals_input() {
  int i=g.modalc;
  while (i-->0) {
    struct modal *modal=g.modalv[i];
    if (modal->defunct) continue;
    int passive=modal->passive;
    if (modal->input) modal->input(modal);
    if (!passive) break;
  }
}

/* Update.
 */
 
void modals_update(double elapsed) {
  int i=g.modalc,top=1;
  while (i-->0) {
    struct modal *modal=g.modalv[i];
    if (modal->defunct) continue;
    if (top) {
      modal->top=1;
      top=0;
    } else {
      modal->top=0;
    }
    if (modal->update) modal->update(modal,elapsed);
    if (!modal->passive) break;
  }
  for (i=g.modalc;i-->0;) {
    struct modal *modal=g.modalv[i];
    if (!modal->defunct) continue;
    g.modalc--;
    memmove(g.modalv+i,g.modalv+i+1,sizeof(void*)*(g.modalc-i));
    modal_del(modal);
  }
}

/* Render.
 */
 
void modals_render() {
  int opaquep=-1,i=g.modalc;
  while (i-->0) {
    struct modal *modal=g.modalv[i];
    if (modal->defunct) continue;
    if (modal->opaque) {
      opaquep=i;
      break;
    }
  }
  if (opaquep<0) { // Blackout first if we don't have an opaque layer. (eg if we have no layers at all)
    graf_draw_rect(&g.graf,0,0,FBW,FBH,0x000000ff);
    opaquep=0;
  }
  for (i=opaquep;i<g.modalc;i++) {
    struct modal *modal=g.modalv[i];
    if (modal->defunct) continue;
    if (modal->render) modal->render(modal);
  }
}

/* Delete modal.
 * Don't touch the stack.
 */

void modal_del(struct modal *modal) {
  if (!modal) return;
  if (modal->del) modal->del(modal);
  free(modal);
}

/* New modal. Pushes to top of stack.
 */
 
struct modal *modal_new(int len) {
  if (g.modalc>=MODAL_LIMIT) return 0;
  if (len<(int)sizeof(struct modal)) return 0;
  struct modal *modal=calloc(1,len);
  if (!modal) return 0;
  g.modalv[g.modalc++]=modal;
  return modal;
}
