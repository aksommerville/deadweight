/* modal.h
 * Top level of our UI is organized into modal layers.
 * These are fixed-size objects that should mostly just dispatch to their more specific controller.
 * There's a global stack of modals at (g.modalv); that's managed here.
 */
 
#ifndef MODAL_H
#define MODAL_H

#define MODAL_LIMIT 8 /* Arbitrary. */

struct modal {
  const char *name; // For logging etc.
  void (*del)(struct modal *modal);
  void (*input)(struct modal *modal);
  void (*update)(struct modal *modal,double elapsed);
  void (*render)(struct modal *modal);
  int opaque; // Nonzero if layers under me can skip render.
  int passive; // Nonzero if layers under me should continue updating (down to the topmost non-passive).
  int top; // Set *by framework* nonzero if this modal is top of stack.
  int defunct; // Nonzero to delete this modal at the next opportunity.
};

/* Hooks for main.c.
 */
void modals_input();
void modals_update(double elapsed);
void modals_render();

/* Primitives, not expected to be needed outside the modal framework.
 * modal_new() pushes on the stack, and fails if stack full.
 */
void modal_del(struct modal *modal);
struct modal *modal_new(int len);

/* Public constructors.
 */
struct modal *modal_new_hello();
struct modal *modal_new_play();
struct modal *modal_new_gameover();
struct modal *modal_new_pause(int x,int y); // Animates popping up from (x,y), presumably the hero's position.
struct modal *modal_new_dialogue();

#endif
