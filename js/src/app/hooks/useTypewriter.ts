import { useEffect, useRef, useState } from "react";

type Mode = "typing" | "hold" | "deleting" | "gap";

export function useTypewriter(messages: string[], tick: number) {
  const [text, setText] = useState("");
  const stateRef = useRef<{ mode: Mode; index: number; char: number; cooldownMs: number }>({
    mode: "typing",
    index: 0,
    char: 0,
    cooldownMs: 0,
  });

  const TYPE_DELAY = 70;
  const DELETE_DELAY = 45;
  const HOLD_AFTER_TYPE = 900;
  const HOLD_AFTER_DELETE = 400;

  useEffect(() => {
    const s = stateRef.current;
    s.cooldownMs -= 33;
    if (s.cooldownMs > 0) return;
    const msg = messages[(s.index % Math.max(1, messages.length))] || "";
    switch (s.mode) {
      case "typing": {
        if (s.char < msg.length) {
          const ch = msg.charAt(s.char);
          setText((t) => t + ch);
          s.char++;
          s.cooldownMs = TYPE_DELAY;
        } else {
          s.mode = "hold";
          s.cooldownMs = HOLD_AFTER_TYPE;
        }
        break;
      }
      case "hold": {
        s.mode = "deleting";
        s.cooldownMs = DELETE_DELAY;
        break;
      }
      case "deleting": {
        if (s.char > 0) {
          setText((t) => t.slice(0, -1));
          s.char--;
          s.cooldownMs = DELETE_DELAY;
        } else {
          s.mode = "gap";
          s.cooldownMs = HOLD_AFTER_DELETE;
        }
        break;
      }
      case "gap": {
        s.index = (s.index + 1) % Math.max(1, messages.length);
        s.mode = "typing";
        s.char = 0;
        s.cooldownMs = TYPE_DELAY;
        setText("");
        break;
      }
    }
  }, [tick, messages]);

  return text;
}


