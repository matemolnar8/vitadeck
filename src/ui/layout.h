#ifndef LAYOUT_H
#define LAYOUT_H

#include "arena.h"
#include "instance_tree.h"

// Fills in every instance's layout from its props; a Text is as large as its lines. Roots, and children of a Rect,
// sit at their own x/y, except that each Text child of a Rect sits one line lower for every Text child before it.
// A Stack or Scroll flows its children along its direction: a child's main-axis coordinate is a leading margin, its
// cross-axis coordinate an offset from the aligned position.
// A Scroll that is the last child and has no main-axis size fills the rest of a parent with a stated one.
// Measures text through the glyph cache, so only the UI thread may call it. A Text's lines go in arena, which must
// outlive the tree's layout.
void layout_tree(ReactInstance **roots, Arena *arena);

#endif /* LAYOUT_H */
