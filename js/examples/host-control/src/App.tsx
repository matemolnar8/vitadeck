import { systemStatsPlugin, type SystemStats } from "@vitadeck/host-plugin-system-stats";
import { Button, Rect, Screen, Stack, Text, insetContent, useTheme, type Color } from "@vitadeck/sdk";
import {
  corePlugin,
  createHostClient,
  linkedHosts,
  type HostCapabilities,
  type HostResult,
  type LinkedHost,
} from "@vitadeck/sdk/host-control";
import React, { useEffect, useState } from "react";

const POLL_MS = 1000;
const LIST_W = 300;
const GAUGE_W = 560;

// Links change from the Shell while the Deck App runs, so they are read again every second.
function useLinkedHosts(): LinkedHost[] {
  const [hosts, setHosts] = useState(linkedHosts);
  useEffect(() => {
    const id = setInterval(() => {
      const next = linkedHosts();
      setHosts((current) => (JSON.stringify(current) === JSON.stringify(next) ? current : next));
    }, POLL_MS);
    return () => {
      clearInterval(id);
    };
  }, []);
  return hosts;
}

// One request at a time, so a slow computer never piles up requests. Returns a stop function.
function pollSystemStats(hostId: string, onResult: (result: HostResult<SystemStats>) => void): () => void {
  const client = createHostClient(systemStatsPlugin, { hostId });
  let stopped = false;
  let timer: number | undefined;
  const poll = async () => {
    const result = await client.read();
    if (stopped) return;
    onResult(result);
    timer = setTimeout(() => void poll(), POLL_MS);
  };
  void poll();
  return () => {
    stopped = true;
    if (timer !== undefined) clearTimeout(timer);
  };
}

function useSystemStats(hosts: LinkedHost[]): Record<string, HostResult<SystemStats>> {
  const [stats, setStats] = useState<Record<string, HostResult<SystemStats>>>({});
  const hostIds = hosts.map((host) => host.id).join(",");
  useEffect(() => {
    const stops = (hostIds === "" ? [] : hostIds.split(",")).map((id) =>
      pollSystemStats(id, (result) => {
        setStats((current) => ({ ...current, [id]: result }));
      }),
    );
    return () => {
      for (const stop of stops) stop();
    };
  }, [hostIds]);
  return stats;
}

function Gauge({ label, percent, detail }: { label: string; percent: number; detail: string }) {
  const { theme } = useTheme();
  const fill: Color = percent >= 85 ? theme.danger : percent >= 60 ? theme.warning : theme.success;
  return (
    <Stack gap={6}>
      <Text fontSize={18} color={theme.text}>
        {`${label}  ${percent.toFixed(1)}%  ${detail}`}
      </Text>
      <Rect width={GAUGE_W} height={22} color={theme.surface} borderRadius={4}>
        <Rect
          width={Math.max(4, Math.round((GAUGE_W * Math.min(percent, 100)) / 100))}
          height={22}
          color={fill}
          borderRadius={4}
        />
      </Rect>
    </Stack>
  );
}

function failureHint(result: HostResult<object>): string {
  if (result.ok) return "";
  if (result.code === "unknown_plugin")
    return "Load @vitadeck/host-plugin-system-stats in that computer's Host Companion.";
  if (result.code === "unreachable" || result.code === "timeout")
    return "Is the Host Companion running? pnpm --dir js host-companion start";
  return "";
}

function HostDetails({ host, stats }: { host: LinkedHost; stats: HostResult<SystemStats> | undefined }) {
  const { theme } = useTheme();
  const [capabilities, setCapabilities] = useState<HostResult<HostCapabilities>>();
  const [echo, setEcho] = useState("Press Echo to time a round trip.");

  useEffect(() => {
    let stopped = false;
    void (async () => {
      const result = await createHostClient(corePlugin, { hostId: host.id }).capabilities();
      if (!stopped) setCapabilities(result);
    })();
    return () => {
      stopped = true;
    };
  }, [host.id]);

  const sendEcho = async () => {
    const started = getTime();
    const reply = await createHostClient(corePlugin, { hostId: host.id }).echo({ text: "Hello from the Vita" });
    const ms = Math.round((getTime() - started) * 1000);
    setEcho(reply.ok ? `"${reply.text}" came back in ${ms} ms.` : `${reply.code}: ${reply.message}`);
  };

  const plugins = capabilities?.ok
    ? capabilities.plugins.map((plugin) => `${plugin.id} (${plugin.commands.join(", ")})`).join("  ")
    : "";

  return (
    <Stack gap={14}>
      <Text fontSize={26} color={theme.text}>
        {host.name}
      </Text>
      <Text fontSize={16} color={theme.outline}>
        {host.url}
      </Text>
      <Text fontSize={16} color={theme.text} width={GAUGE_W} wrap="word">
        {capabilities === undefined
          ? "Asking for capabilities..."
          : capabilities.ok
            ? `${capabilities.platform}, protocol ${capabilities.protocolVersion}. Plugins: ${plugins}`
            : `${capabilities.code}: ${capabilities.message}`}
      </Text>
      {stats === undefined ? (
        <Text fontSize={18} color={theme.outline}>
          Reading system stats...
        </Text>
      ) : stats.ok ? (
        <Stack gap={14}>
          <Gauge label="CPU" percent={stats.cpuPercent} detail="" />
          <Gauge
            label="Memory"
            percent={stats.memoryUsedPercent}
            detail={`of ${Math.round(stats.memoryTotalBytes / 1024 ** 3)} GiB`}
          />
          <Text fontSize={16} color={theme.outline}>
            {`Up ${Math.floor(stats.uptimeSeconds / 3600)} h ${Math.floor((stats.uptimeSeconds % 3600) / 60)} min`}
          </Text>
        </Stack>
      ) : (
        <Text fontSize={18} color={theme.warning} width={GAUGE_W} wrap="word">
          {`${stats.code}: ${stats.message}\n${failureHint(stats)}`}
        </Text>
      )}
      <Stack direction="row" gap={12} align="center">
        <Button
          width={120}
          height={40}
          label="Echo"
          onPress={() => void sendEcho()}
          color={theme.buttonBackground}
          textColor={theme.buttonText}
          borderRadius={4}
        />
        <Text fontSize={16} color={theme.text} width={GAUGE_W - 140} wrap="word">
          {echo}
        </Text>
      </Stack>
    </Stack>
  );
}

export default function HostControlDeckApp() {
  const { theme } = useTheme();
  const hosts = useLinkedHosts();
  const stats = useSystemStats(hosts);
  const [selectedId, setSelectedId] = useState<string>();
  const selected = hosts.find((host) => host.id === selectedId) ?? hosts[0];
  const { x, y, width, height } = insetContent();

  return (
    <Screen color={theme.background}>
      <Stack x={x} y={y} width={width} height={height} direction="row" gap={20}>
        <Stack width={LIST_W} gap={10}>
          <Text fontSize={24} color={theme.text}>
            Linked computers
          </Text>
          {hosts.map((host, index) => {
            const result = stats[host.id];
            const load = result?.ok ? `${Math.round(result.cpuPercent)}%` : result ? "offline" : "...";
            return (
              <Button
                key={host.id}
                width={LIST_W}
                height={48}
                label={`${host.name}   ${load}`}
                align="left"
                autoFocus={index === 0}
                onPress={() => {
                  setSelectedId(host.id);
                }}
                color={host.id === selected?.id ? theme.primary : theme.surfaceAlt}
                textColor={theme.text}
                borderRadius={4}
              />
            );
          })}
          {hosts.length === 0 && (
            <Text fontSize={16} color={theme.outline} width={LIST_W} wrap="word">
              {
                "No computer is linked.\n\n1. On the computer: pnpm --dir js host-companion start\n2. Press Start, open Host Control in the Shell.\n3. On the computer, run the link command it shows."
              }
            </Text>
          )}
        </Stack>
        <Rect width={width - LIST_W - 20} height={height} color={theme.surfaceAlt} borderRadius={6}>
          <Stack x={20} y={20}>
            {selected ? (
              <HostDetails key={selected.id} host={selected} stats={stats[selected.id]} />
            ) : (
              <Text fontSize={20} color={theme.outline}>
                Link a computer to see its stats here.
              </Text>
            )}
          </Stack>
        </Rect>
      </Stack>
    </Screen>
  );
}
