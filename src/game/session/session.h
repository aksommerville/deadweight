/* session.h
 * The session is the game world, basically everything that happens between visits to the hello modal.
 * We use globals extensively; there can not be multiple sessions.
 */
 
#ifndef SESSION_H
#define SESSION_H

/* Just once. main.c will call.
 */
int session_init();

/* Start of each session. Hello modal will call.
 */
int session_reset();

int enter_map(int rid,int transition);

struct listener {
  int listenerid;
  int k;
  void *userdata;
  void (*cb)(int k,int v,void *userdata);
};

int store_listen(int k,void (*cb)(int k,int v,void *userdata),void *userdata); // => listenerid or <=0
void store_unlisten(int listenerid);

int store_get(int k);
int store_set(int k,int v); // => adjusted value

#endif
