/* sprite.h
 */
 
#ifndef SPRITE_H
#define SPRITE_H

struct sprite;
struct sprite_type;

struct sprdef {
  int rid;
  const void *cmdv;
  int cmdc;
  const struct sprite_type *type;
};

struct sprite {
  const struct sprite_type *type;
  int defunct;
  double x,y;
  uint8_t tileid;
  uint8_t xform;
  uint8_t layer; // default 128
  uint32_t arg;
  const void *cmdv;
  int cmdc;
  int solid; // Participates in physics.
  int airborne; // "hole" cell physics are passable.
  double phl,phr,pht,phb; // Physical bounds relative to (x,y). (phl,pht) are typically negative and (phr,phb) positive.
  int summoning; // Hero sets for the one sprite being summoned with the wand.
  int decorative; // Disables most interactions.
};

struct sprite_type {
  const char *name;
  int objlen;
  void (*del)(struct sprite *sprite);
  int (*init)(struct sprite *sprite);
  void (*update)(struct sprite *sprite,double elapsed);
  void (*render)(struct sprite *sprite,int x,int y);
  void (*hurt)(struct sprite *sprite,struct sprite *assailant); // Sprites that take damage at all must implement.
};

// Session calls this once per resource at startup.
int load_sprite(int rid,const void *src,int srcc);

/* Let the framework call these; outside users should not.
 * sprite_new does not call init.
 */
void sprite_del(struct sprite *sprite);
struct sprite *sprite_new(const struct sprite_type *type);

struct sprite *sprite_spawn(double x,double y,uint16_t rid,const struct sprite_type *type,uint32_t arg);

void sprites_kill_all();
void sprites_delete_volatile(); // Immediately delete all except (g.hero,g.princess). DO NOT CALL DURING ITERATION.

void sprites_update(double elapsed);
void sprites_render(int offx,int offy);
void sprites_render_volatile(); // Skip hero and princess, and no offset.

const struct sprite_type *sprite_type_by_id(uint16_t sprtype);

#define _(tag) extern const struct sprite_type sprite_type_##tag;
SPRTYPE_FOR_EACH
#undef _

const struct sprdef *sprdef_by_id(int rid);

/* If the princess exists and is on screen, monsters like to attack her.
 * Otherwise the hero.
 */
struct sprite *get_preferred_monster_target();

/* Impulse physics: When a solid sprite moves, we immediately resolve all collisions.
 * These both work for non-solid sprites, but are trivial, you might as well update (x,y) yourself.
 * "move" takes a delta and assumes that the current position is valid.
 * "warp" takes an absolute position and puts you somewhere valid as near as possible to that.
 * Both return 0 if no motion, 2 if placed exactly as requested, or 1 if adjusted.
 * Move may skip intervening blockages if the delta is too wide, keep it small.
 * If warp returns zero, it leaves (sprite) in an undefined position. You must put it somewhere else.
 */
int sprite_move(struct sprite *sprite,double dx,double dy);
int sprite_warp(struct sprite *sprite,double x,double y);

int sprite_position_valid(const struct sprite *sprite);

// Signal from the modal stack that something was pushed. So hero can cancel in-progress actions.
void sprite_hero_losing_focus(struct sprite *sprite);

void sprite_hero_force_broom(struct sprite *sprite);

void sprite_smoke_setup(struct sprite *sprite,double dx,double dy);

#endif
