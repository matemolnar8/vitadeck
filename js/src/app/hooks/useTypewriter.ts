import { useEffect, useRef, useState } from "react";

type Mode = "typing" | "hold" | "deleting" | "gap";

export function useTypewriter(messages: string[]) {
  const [text, setText] = useState("");
  const [cursorOn, setCursorOn] = useState(true);
  const stateRef = useRef<{
    mode: Mode;
    index: number;
    char: number;
    cooldownMs: number;
  }>({
    mode: "typing",
    index: 0,
    char: 0,
    cooldownMs: 0,
  });
  const messagesRef = useRef(messages);
  useEffect(() => {
    messagesRef.current = messages;
  }, [messages]);

  const TYPE_DELAY = 70;
  const DELETE_DELAY = 45;
  const HOLD_AFTER_TYPE = 900;
  const HOLD_AFTER_DELETE = 400;

  useEffect(() => {
    let elapsedMs = 0;
    let cursorElapsed = 0;
    const intervalId = setInterval(() => {
      // Advance clock
      elapsedMs += 33;
      cursorElapsed += 33;

      // Cursor blink
      if (cursorElapsed >= 420) {
        setCursorOn((v) => !v);
        cursorElapsed = 0;
      }

      // Typewriter FSM
      const s = stateRef.current;
      s.cooldownMs -= 33;
      if (s.cooldownMs > 0) return;
      const list = messagesRef.current;
      const msg = list[s.index % Math.max(1, list.length)] || "";
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
          s.index = (s.index + 1) % Math.max(1, list.length);
          s.mode = "typing";
          s.char = 0;
          s.cooldownMs = TYPE_DELAY;
          setText("");
          break;
        }
      }
    }, 33);
    return () => clearInterval(intervalId);
  }, []);

  return { text, cursorOn };
}
