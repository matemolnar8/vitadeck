import { Button, Rect, Screen, Text, insetContent, useTheme } from "@vitadeck/sdk";
import React, { useCallback, useEffect, useState } from "react";

type CatFact = { fact: string; length: number };

type HttpBinEcho = { json: { client: string; count: number } | null };

export default function FetchDeckApp() {
  const { theme } = useTheme();
  const { x, y, width, height } = insetContent();
  const gap = 16;
  const rowH = (height - gap) / 2;

  const [fact, setFact] = useState('Press "New fact" to call catfact.ninja');
  const [factStatus, setFactStatus] = useState("idle");
  const [factLoading, setFactLoading] = useState(false);

  const [echo, setEcho] = useState('Press "POST echo" to send JSON to httpbin.org');
  const [echoStatus, setEchoStatus] = useState("idle");
  const [echoLoading, setEchoLoading] = useState(false);
  const [count, setCount] = useState(0);

  const loadFact = useCallback(async () => {
    setFactLoading(true);
    setFactStatus("loading…");
    try {
      const res = await fetch("https://catfact.ninja/fact");
      setFactStatus(`${res.status} ${res.statusText}`);
      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      const data = await res.json<CatFact>();
      setFact(data.fact);
    } catch (err) {
      setFact(`Error: ${String(err)}`);
      setFactStatus("error");
    } finally {
      setFactLoading(false);
    }
  }, []);

  const postEcho = useCallback(async () => {
    setEchoLoading(true);
    setEchoStatus("loading…");
    const next = count + 1;
    setCount(next);
    try {
      const res = await fetch("https://httpbin.org/post", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ client: "vitadeck", count: next }),
      });
      setEchoStatus(`${res.status} ${res.statusText}`);
      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      const data = await res.json<HttpBinEcho>();
      setEcho(`Server echoed: ${JSON.stringify(data.json)}`);
    } catch (err) {
      setEcho(`Error: ${String(err)}`);
      setEchoStatus("error");
    } finally {
      setEchoLoading(false);
    }
  }, [count]);

  useEffect(() => {
    loadFact();
  }, [loadFact]);

  return (
    <Screen>
      <Rect x={x} y={y} width={width} height={rowH} color={theme.surface} borderRadius={0.06}>
        <Text fontSize={22} color={theme.text}>
          GET catfact.ninja — {factStatus}
        </Text>
        <Text fontSize={18} color={theme.outline}>
          {fact}
        </Text>
        <Button
          x={20}
          y={rowH - 56}
          width={200}
          height={40}
          label={factLoading ? "Loading…" : "New fact"}
          onPress={() => {
            if (!factLoading) loadFact();
          }}
          color={theme.success}
          textColor={theme.navText}
          borderRadius={0.12}
        />
      </Rect>

      <Rect x={x} y={y + rowH + gap} width={width} height={rowH} color={theme.surfaceAlt} borderRadius={0.06}>
        <Text fontSize={22} color={theme.text}>
          POST httpbin.org — {echoStatus}
        </Text>
        <Text fontSize={18} color={theme.outline}>
          {echo}
        </Text>
        <Button
          x={20}
          y={rowH - 56}
          width={200}
          height={40}
          label={echoLoading ? "Sending…" : "POST echo"}
          onPress={() => {
            if (!echoLoading) postEcho();
          }}
          color={theme.accent}
          textColor={theme.navText}
          borderRadius={0.12}
        />
      </Rect>
    </Screen>
  );
}
