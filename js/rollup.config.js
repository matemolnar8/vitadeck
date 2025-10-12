import { defineConfig } from "rollup";
import typescript from "@rollup/plugin-typescript";
import nodeResolve from "@rollup/plugin-node-resolve";
import commonjs from "@rollup/plugin-commonjs";
import replace from "@rollup/plugin-replace";
import { corejsPlugin } from "rollup-plugin-corejs";

export default defineConfig({
  input: {
    main: "./src/main.tsx",
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
  plugins: [
    typescript(),
    nodeResolve(),
    replace({
      'process.env.NODE_ENV': JSON.stringify('development'),
      preventAssignment: true,
    }),
    commonjs(),
    corejsPlugin(),
  ],
});
