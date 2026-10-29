import os from "node:os";
import { implementHostPlugin } from "@vitadeck/sdk/host-control";
import { hostPlugin } from "./plugin";

// Runs in the Host Companion on the computer, with Node.js and the permissions of the user who started it.
// `vitadeck build` bundles it into dist/vitadeck-scaffold-template.vdplugin.
export default implementHostPlugin(hostPlugin, {
  greet: ({ name }) => ({ message: `Hello ${name}, this is ${os.hostname()}.` }),
});
