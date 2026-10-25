#include <math.h>
#include <string.h>
#include "focus_nav.h"

int focus_nav_pick(const FocusableElement *elems, int count, const char *focused_id, NavDirection dir)
{
    if (!elems || count == 0) return -1;

    int cur_cx = 0, cur_cy = 0;
    bool has_current = false;

    // Find current focused element center
    if (focused_id) {
        for (int i = 0; i < count; i++) {
            if (strcmp(elems[i].id, focused_id) == 0) {
                cur_cx = elems[i].x + elems[i].width / 2;
                cur_cy = elems[i].y + elems[i].height / 2;
                has_current = true;
                break;
            }
        }
    }

    if (!has_current) return 0;

    int best = -1;
    float best_score = 1e9f;

    for (int i = 0; i < count; i++) {
        if (strcmp(elems[i].id, focused_id) == 0) continue;

        int cx = elems[i].x + elems[i].width / 2;
        int cy = elems[i].y + elems[i].height / 2;
        int dx = cx - cur_cx;
        int dy = cy - cur_cy;

        // Check if element is in the correct direction
        bool valid = false;
        switch (dir) {
        case NAV_UP:
            valid = dy < 0;
            break;
        case NAV_DOWN:
            valid = dy > 0;
            break;
        case NAV_LEFT:
            valid = dx < 0;
            break;
        case NAV_RIGHT:
            valid = dx > 0;
            break;
        }
        if (!valid) continue;

        // Calculate weighted distance (favor elements more aligned with direction)
        float primary = 0, secondary = 0;
        switch (dir) {
        case NAV_UP:
        case NAV_DOWN:
            primary = fabsf((float)dy);
            secondary = fabsf((float)dx);
            break;
        case NAV_LEFT:
        case NAV_RIGHT:
            primary = fabsf((float)dx);
            secondary = fabsf((float)dy);
            break;
        }
        float score = primary + secondary * 2.0f;

        if (score < best_score) {
            best_score = score;
            best = i;
        }
    }

    return best;
}
