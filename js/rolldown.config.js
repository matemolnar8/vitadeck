import babel from "@rollup/plugin-babel";
import path from "node:path";
import { defineConfig } from "rolldown";

const isProd = process.env.NODE_ENV === "production";

export default defineConfig({
  input: {
    main: "./src/main.tsx",
  },
  output: {
    dir: "dist",
    format: "iife",
    name: "vitadeck",
    minify: isProd,
  },
  resolve: {
    alias: {
      "@vitadeck/active-deck-app": path.resolve("src/generated/active-deck-app.ts"),
      "@vitadeck/runtime": path.resolve("src/runtime/index.tsx"),
      react: path.resolve("node_modules/react/index.js"),
      "react-compiler-runtime": path.resolve("node_modules/react-compiler-runtime/dist/index.js"),
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
      presets: [
        "@babel/preset-react",
        ["@babel/preset-typescript", { isTSX: true, allExtensions: true }],
      ],
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
                    console.error(
                      `  🔍 ${relativePath}:${firstError.loc.start.line}: ${firstError.message}`,
                    );
                  }
                }
              },
            },
          },
        ],
      ],
    }),
  ],
});
