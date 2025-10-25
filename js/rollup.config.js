import { defineConfig } from "rollup";
import typescript from "@rollup/plugin-typescript";
import nodeResolve from "@rollup/plugin-node-resolve";
import commonjs from "@rollup/plugin-commonjs";
import replace from "@rollup/plugin-replace";
import babel from "@rollup/plugin-babel";
import { corejsPlugin } from "rollup-plugin-corejs";

/*

TypeScript ES2015 -> Babel Es

*/

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
    commonjs(),
    nodeResolve(),
    babel({
      babelHelpers: "bundled",
      extensions: [".ts", ".tsx", ".js", ".jsx"],
      plugins: [
        [
          "babel-plugin-react-compiler",
          {
            target: "18",
          },
        ],
      ],
      exclude: [/node_modules\/core-js/],
      presets: [
        [
          "@babel/preset-env",
          {
            targets: { ie: "10" },
            bugfixes: true,
            modules: false,
          },
        ],
      ],
    }),
    replace({
      "process.env.NODE_ENV": JSON.stringify("development"),
      preventAssignment: true,
    }),
    corejsPlugin(),
  ],
});
