#ifndef VD_BLUR_H
#define VD_BLUR_H

// Draws what is painted between blur_begin and blur_end to the screen, blurred, e.g. the Deck App behind the Shell.
void blur_begin(void);
void blur_end(void);
// Before CloseWindow.
void blur_unload(void);

#endif /* VD_BLUR_H */
