import { transformAsync } from "@babel/core";
import type { Plugin } from "rolldown";

// Babel 8 defaults are pinned back to Babel 7 output: classic JSX, no dev JSX transform, unused imports elided.
export function babel(): Plugin {
  return {
    name: "vitadeck:babel",
    transform: {
      filter: { id: /\.[jt]sx?(\?.*)?(#.*)?$/u },
      async handler(code, id) {
        const result = await transformAsync(code, {
          filename: id,
          caller: {
            name: "vitadeck",
            supportsStaticESM: true,
            supportsDynamicImport: true,
            supportsTopLevelAwait: true,
            supportsExportNamespaceFrom: true,
          },
          presets: [
            ["@babel/preset-react", { runtime: "classic", development: false }],
            ["@babel/preset-typescript", { ignoreExtensions: true, onlyRemoveTypeImports: false }],
          ],
          plugins: [["babel-plugin-react-compiler", { target: "18" }]],
        });
        return typeof result?.code === "string" ? { code: result.code } : null;
      },
    },
  };
}
