import { defineConfig } from "rollup";
import typescript from "@rollup/plugin-typescript";

export default defineConfig({
  input: {
    main: "./main.ts",
  },
  output: {
    dir: "dist",
    format: "iife",
    name: "vitadeck",
    generatedCode: {
      preset: "es5",
      symbols: false,
    },
  },
  plugins: [typescript()],
});
