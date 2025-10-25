import { defineConfig } from "rollup";
import typescript from "@rollup/plugin-typescript";
import nodeResolve from "@rollup/plugin-node-resolve";
import commonjs from "@rollup/plugin-commonjs";
import replace from "@rollup/plugin-replace";
import babel from "@rollup/plugin-babel";
import { corejsPlugin } from "rollup-plugin-corejs";
import path from "path";

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
            logger: {
              logEvent: (absoluteFilePath, event) => {
                const relativePath = path.relative(
                  process.cwd(),
                  absoluteFilePath
                );
                if (event.kind === "CompileSuccess") {
                  console.log(`[React Compiler] ✅ ${relativePath}`);
                } else if (event.kind === "CompileError") {
                  const { detail } = event;
                  const { reason, category, description } =
                    detail.options || detail;
                  console.error(
                    `[React Compiler] ❌ ${relativePath} | 🚫 (${category}) ${reason}`
                  );
                  if (description) {
                    console.error(`  📝 ${description}`);
                  }
                  if (detail.options?.details?.[0]) {
                    const firstError = detail.options.details[0];
                    console.error(
                      `  🔍 ${relativePath}:${firstError.loc.start.line}: ${firstError.message}`
                    );
                  }
                }
              },
            },
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
