import { defineConfig } from "rolldown";

export default defineConfig({
  input: "./cli/cli.ts",
  output: {
    file: "./dist/cli.mjs",
    format: "esm",
    banner: "#!/usr/bin/env node\n",
  },
  platform: "node",
  external: (id) =>
    id === "rolldown" ||
    id === "@rollup/plugin-babel" ||
    id === "babel-plugin-react-compiler" ||
    id.startsWith("@babel/"),
  tsconfig: false,
});
