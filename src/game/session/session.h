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

#endif
