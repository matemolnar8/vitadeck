#ifndef TEXT_H
#define TEXT_H

#include <raylib.h>
#include "fonts.h"
#include "instance_tree.h"

#define TEXT_LAYOUT_MAX_LINES 128
#define TEXT_LAYOUT_MAX_LINE_CHARS 512

// Single-line text: text_lines breaks a Text instance into lines, and control characters draw nothing. Colour emoji
// draw untinted from the font's line; .fnt and .bdf fonts have none.
float text_measure(const VdFont *font, const char *text, int font_size);
// What the cluster at text, an emoji or one codepoint, adds to text_measure; sets length to its bytes.
float text_cluster_width(const VdFont *font, const char *text, int font_size, int *length);
int text_line_height(const VdFont *font, int font_size);
// position is the top-left of the line box.
void text_draw(const VdFont *font, const char *text, Vector2 position, int font_size, Color color);

typedef struct {
    char lines[TEXT_LAYOUT_MAX_LINES][TEXT_LAYOUT_MAX_LINE_CHARS];
    int widths[TEXT_LAYOUT_MAX_LINES];
    int count;
    // The widest line, or the Text's width when that is wider.
    int width;
} TextLines;

int text_props_font_size(const TextProps *text);
// The distance between line tops: lineHeight, else the font's line height plus 4.
int text_props_line_height(const TextProps *text);
// A Text instance's raw text children joined and broken into lines: at newlines, and with a width and word wrap
// between words. Layout sizes the Text from them and render draws them. The result is overwritten by the next call.
const TextLines *text_lines(const ReactInstance *text);

// Each frame: text_begin_frame, then text_measure every string the frame draws, then text_upload_pages, then draw.
// Glyphs first cached after the upload show from the next frame.
void text_begin_frame(void);
void text_upload_pages(void);
// Drops every cached glyph and atlas texture.
void text_reset(void);

#endif /* TEXT_H */
