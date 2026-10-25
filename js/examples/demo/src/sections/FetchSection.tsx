import { Button, Stack, useTheme } from "@vitadeck/sdk";
import React, { useEffect, useState } from "react";
import { Label, Section } from "../ui";

const REPO_URL = "https://api.github.com/repos/matemolnar8/vitadeck";

type Repo = { full_name: string; description: string | null; stargazers_count: number; open_issues_count: number };

type RequestState = { status: "loading" } | { status: "done"; repo: Repo } | { status: "error"; message: string };

function isRepo(value: unknown): value is Repo {
  if (typeof value !== "object" || value === null) return false;
  return (
    "full_name" in value &&
    typeof value.full_name === "string" &&
    "description" in value &&
    (typeof value.description === "string" || value.description === null) &&
    "stargazers_count" in value &&
    typeof value.stargazers_count === "number" &&
    "open_issues_count" in value &&
    typeof value.open_issues_count === "number"
  );
}

async function loadRepo(): Promise<Repo> {
  const response = await fetch(REPO_URL, { headers: { Accept: "application/vnd.github+json" } });
  if (!response.ok) throw new Error(`HTTP ${response.status} ${response.statusText}`);
  const body = await response.json();
  if (!isRepo(body)) throw new Error("Unexpected response shape");
  return body;
}

export function FetchSection() {
  const { theme } = useTheme();
  const [attempt, setAttempt] = useState(0);
  const [request, setRequest] = useState<RequestState>({ status: "loading" });

  useEffect(() => {
    let ignore = false;
    const run = async () => {
      // Without this catch, a failed request (offline, DNS, TLS) would be an unhandled rejection and fail the Deck App.
      try {
        const repo = await loadRepo();
        if (!ignore) setRequest({ status: "done", repo });
      } catch (error) {
        if (!ignore) setRequest({ status: "error", message: error instanceof Error ? error.message : String(error) });
      }
    };
    void run();
    return () => {
      ignore = true;
    };
  }, [attempt]);

  return (
    <Section
      title="fetch"
      blurb={`fetch(url, init) returns a Promise of a response with status, headers, text() and json(). This section GETs ${REPO_URL} when it opens.`}
    >
      {request.status === "loading" && <Label fontSize={20}>Loading…</Label>}
      {request.status === "done" && (
        <Stack gap={6}>
          <Label fontSize={22} color={theme.success}>
            {request.repo.full_name}
          </Label>
          <Label lines={2}>{request.repo.description ?? "No description"}</Label>
          <Label>{`★ ${request.repo.stargazers_count} stars · ${request.repo.open_issues_count} open issues`}</Label>
        </Stack>
      )}
      {request.status === "error" && (
        <Stack gap={6}>
          <Label fontSize={20} color={theme.danger}>{`Request failed: ${request.message}`}</Label>
          <Label fontSize={14} color={theme.outline}>
            Offline? Connect the Vita to Wi-Fi and try again.
          </Label>
        </Stack>
      )}
      <Button
        width={200}
        height={44}
        label={request.status === "loading" ? "Loading…" : "Fetch again"}
        color={theme.buttonBackground}
        textColor={theme.buttonText}
        borderRadius={8}
        autoFocus
        onPress={() => {
          setRequest({ status: "loading" });
          setAttempt((count) => count + 1);
        }}
      />
    </Section>
  );
}
