type InteractiveRect = {
  id: string;
  x: number;
  y: number;
  width: number;
  height: number;
  onClick?: () => void;
  onMouseDown?: () => void;
  onMouseUp?: () => void;
  onMouseEnter?: () => void;
  onMouseLeave?: () => void;
};

const MOUSE_BUTTON_LEFT = 0;

// Rectangles registered during this frame (in draw order)
const interactiveRects: InteractiveRect[] = [];

// Persistent state across frames
let lastHoverId: string | undefined = undefined;
let activeTouchRectId: string | undefined = undefined;
let wasTouchDownPrev = false;

export function clearInteractiveRects() {
  interactiveRects.length = 0;
}

export function registerInteractiveRect(rect: InteractiveRect) {
  interactiveRects.push(rect);
}

function findTopmostRectIdAt(x: number, y: number): string | undefined {
  for (let i = interactiveRects.length - 1; i >= 0; i--) {
    const r = interactiveRects[i];
    if (!r) continue;
    if (x >= r.x && x < r.x + r.width && y >= r.y && y < r.y + r.height) {
      return r.id;
    }
  }
  return undefined;
}

function getRectById(id: string | undefined): InteractiveRect | undefined {
  if (!id) return undefined;
  for (let i = interactiveRects.length - 1; i >= 0; i--) {
    const r = interactiveRects[i];
    if (!r) continue;
    if (r.id === id) return r;
  }
  return undefined;
}

export function processMouseInput() {
  const mx = getMouseX();
  const my = getMouseY();

  const currentTopId = findTopmostRectIdAt(mx, my) || "";

  // Hover enter/leave
  if (lastHoverId !== currentTopId) {
    const prev = getRectById(lastHoverId);
    if (prev?.onMouseLeave) {
      try {
        prev.onMouseLeave();
      } catch (e) {
        console.error(e);
      }
    }
    const curr = getRectById(currentTopId);
    if (curr?.onMouseEnter) {
      try {
        curr.onMouseEnter();
      } catch (e) {
        console.error(e);
      }
    }
    lastHoverId = currentTopId || undefined;
  }

  // Mouse down/click on topmost
  if (isMouseButtonPressed(MOUSE_BUTTON_LEFT) && currentTopId) {
    const r = getRectById(currentTopId);
    if (r) {
      try {
        r.onMouseDown?.();
      } catch (e) {
        console.error(e);
      }
      try {
        r.onClick?.();
      } catch (e) {
        console.error(e);
      }
    }
  }

  // Mouse up on topmost
  if (isMouseButtonReleased(MOUSE_BUTTON_LEFT) && currentTopId) {
    const r = getRectById(currentTopId);
    if (r) {
      try {
        r.onMouseUp?.();
      } catch (e) {
        console.error(e);
      }
    }
  }
}

export function processTouchInput() {
  const touchPositions = getTouchPositions();
  const isDown = touchPositions.length > 0;

  // Touch began
  if (!wasTouchDownPrev && isDown) {
    const touch = touchPositions[0]!;
    const topId = findTopmostRectIdAt(touch.x, touch.y);
    if (topId) {
      activeTouchRectId = topId;
      const r = getRectById(activeTouchRectId);
      if (r) {
        try {
          r.onMouseDown?.();
        } catch (e) {
          console.error(e);
        }
        try {
          r.onClick?.();
        } catch (e) {
          console.error(e);
        }
      }
    } else {
      activeTouchRectId = undefined;
    }
  }

  // Touch ended
  if (wasTouchDownPrev && !isDown) {
    if (activeTouchRectId) {
      const r = getRectById(activeTouchRectId);
      if (r) {
        try {
          r.onMouseUp?.();
        } catch (e) {
          console.error(e);
        }
      }
    }
    activeTouchRectId = undefined;
  }

  wasTouchDownPrev = isDown;
}

export function processInput() {
  processMouseInput();
  processTouchInput();
}
