import path from "node:path";
import babel from "@rollup/plugin-babel";
import commonjs from "@rollup/plugin-commonjs";
import nodeResolve from "@rollup/plugin-node-resolve";
import replace from "@rollup/plugin-replace";
import terser from "@rollup/plugin-terser";
import typescript from "@rollup/plugin-typescript";
import { defineConfig } from "rollup";

export default defineConfig({
  input: {
    main: "./src/main.tsx",
  },
  output: {
    dir: "dist",
    format: "iife",
    name: "vitadeck",
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
                const relativePath = path.relative(process.cwd(), absoluteFilePath);
                if (event.kind === "CompileSuccess") {
                  console.log(`[React Compiler] ✅ ${relativePath}`);
                } else if (event.kind === "CompileError") {
                  const { detail } = event;
                  const { reason, category, description } = detail.options || detail;
                  console.error(`[React Compiler] ❌ ${relativePath} | 🚫 (${category}) ${reason}`);
                  if (description) {
                    console.error(`  📝 ${description}`);
                  }
                  if (detail.options?.details?.[0]) {
                    const firstError = detail.options.details[0];
                    console.error(`  🔍 ${relativePath}:${firstError.loc.start.line}: ${firstError.message}`);
                  }
                }
              },
            },
          },
        ],
      ],
    }),
    replace({
      "process.env.NODE_ENV": JSON.stringify(process.env.NODE_ENV || "development"),
      preventAssignment: true,
    }),
    process.env.NODE_ENV === "production"
      ? terser({
          ecma: 2020,
        })
      : null,
  ],
});
