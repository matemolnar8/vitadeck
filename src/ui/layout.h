#ifndef LAYOUT_H
#define LAYOUT_H

#include "instance_tree.h"

// Fills in every instance's layout from its props. Roots, and children of a Rect, sit at their own x/y.
// A Stack or Scroll flows its Rect, Button, Text Input, Image and Stack children along its direction: a child's
// main-axis coordinate is a leading margin, its cross-axis coordinate an offset from the aligned position.
// Other children (Text, a nested Scroll) keep their own x/y, measured from inside the padding.
void layout_tree(ReactInstance **roots);

#endif /* LAYOUT_H */
