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
};

struct sprite_type {
  const char *name;
  int objlen;
  void (*del)(struct sprite *sprite);
  int (*init)(struct sprite *sprite);
  void (*update)(struct sprite *sprite,double elapsed);
  void (*render)(struct sprite *sprite,int x,int y);
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

#endif
