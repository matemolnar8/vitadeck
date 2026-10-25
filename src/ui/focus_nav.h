#ifndef FOCUS_NAV_H
#define FOCUS_NAV_H

#include "instance_tree.h"

typedef enum { NAV_UP, NAV_DOWN, NAV_LEFT, NAV_RIGHT } NavDirection;

// Index into elems of the element D-pad navigation in dir moves to, or -1 when nothing lies that way.
// With no focus, or a focused id that is not in elems, it picks the first element.
int focus_nav_pick(const FocusableElement *elems, int count, const char *focused_id, NavDirection dir);

#endif /* FOCUS_NAV_H */
