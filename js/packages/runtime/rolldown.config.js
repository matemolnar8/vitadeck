import babel from "@rollup/plugin-babel";
import path from "node:path";
import { defineConfig } from "rolldown";

const isProd = process.env.NODE_ENV === "production";

export default defineConfig({
  input: "./src/main.tsx",
  output: {
    file: "../../dist/runtime/runtime.js",
    format: "iife",
    name: "vitadeck",
    minify: isProd,
  },
  resolve: {
    alias: {
      "@vitadeck/sdk": path.resolve("../sdk/dist/index.js"),
      "@vitadeck/sdk/internal": path.resolve("../sdk/dist/internal.js"),
      react: path.resolve("../../node_modules/react/index.js"),
      "react-compiler-runtime": path.resolve("../../node_modules/react-compiler-runtime/dist/index.js"),
    },
  },
  tsconfig: false,
  transform: {
    define: {
      "process.env.NODE_ENV": JSON.stringify(process.env.NODE_ENV || "development"),
    },
  },
  plugins: [
    babel({
      babelHelpers: "bundled",
      extensions: [".ts", ".tsx", ".js", ".jsx"],
      presets: ["@babel/preset-react", ["@babel/preset-typescript", { isTSX: true, allExtensions: true }]],
      plugins: [["babel-plugin-react-compiler", { target: "18" }]],
    }),
  ],
});
