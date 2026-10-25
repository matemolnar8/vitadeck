#ifndef SCROLL_H
#define SCROLL_H

/* Scroll offsets for scroll containers, keyed by instance id.
 * UI-thread only (render + input polling run on the same thread). */

int scroll_get_offset(const char *id);
void scroll_set_offset(const char *id, int offset);

/* Drop all stored offsets (e.g. when the Deck App runtime restarts). */
void scroll_reset(void);

/* Largest offset that keeps the viewport inside the content; 0 when the content fits. */
int scroll_max_offset(int viewport_height, int content_height);

/* Advances a touch momentum scroll by one frame and returns the next offset, clamped to [0, max_offset].
 * Stops the momentum (zeroes *velocity, in px/s) when it reaches either end. Friction is the caller's. */
int scroll_momentum_step(int offset, int max_offset, float *velocity, float dt);

#endif /* SCROLL_H */
