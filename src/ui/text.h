#ifndef TEXT_H
#define TEXT_H

#include <raylib.h>
#include "fonts.h"

// Single-line text: callers break lines themselves, and control characters draw nothing. Colour emoji draw untinted
// from the font's line; .fnt and .bdf fonts have none.
float text_measure(const VdFont *font, const char *text, int font_size);
// What the cluster at text, an emoji or one codepoint, adds to text_measure; sets length to its bytes.
float text_cluster_width(const VdFont *font, const char *text, int font_size, int *length);
int text_line_height(const VdFont *font, int font_size);
// position is the top-left of the line box.
void text_draw(const VdFont *font, const char *text, Vector2 position, int font_size, Color color);

// Each frame: text_begin_frame, then text_measure every string the frame draws, then text_upload_pages, then draw.
// Glyphs first cached after the upload show from the next frame.
void text_begin_frame(void);
void text_upload_pages(void);
// Drops every cached glyph and atlas texture.
void text_reset(void);

#endif /* TEXT_H */
