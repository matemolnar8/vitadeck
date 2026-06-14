#ifndef SCROLL_H
#define SCROLL_H

/* Scroll offsets for scroll containers, keyed by instance id.
 * UI-thread only (render + input polling run on the same thread). */

int scroll_get_offset(const char *id);
void scroll_set_offset(const char *id, int offset);

/* Drop all stored offsets (e.g. when the Deck App runtime restarts). */
void scroll_reset(void);

#endif /* SCROLL_H */
