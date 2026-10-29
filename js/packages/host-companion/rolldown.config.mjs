import { defineConfig } from "rolldown";

// Bundles the SDK's host-control code in, so the published companion has no dependencies.
export default defineConfig({
  input: "./src/cli.ts",
  output: {
    file: "./dist/cli.mjs",
    format: "esm",
    banner: "#!/usr/bin/env node\n",
  },
  platform: "node",
  tsconfig: false,
});
