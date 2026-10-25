/** The Deck App Package Manifest (`manifest.json`) the CLI writes and `src/core/package_manifest.c` validates. */
export type DeckAppPackageManifest = {
  schemaVersion: 1;
  name: string;
  version: string;
  entry: "app.js";
  fonts?: Record<string, string>;
  images?: Record<string, string>;
};
