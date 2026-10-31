// CommonJS, like many npm dependencies. Bundled, its require of a Node built-in becomes createRequire(import.meta.url).
/* oxlint-disable typescript/no-unsafe-assignment, typescript/no-unsafe-call, typescript/no-unsafe-member-access */
const { EOL } = require("node:os");

exports.greeting = (name) => `Hello, ${name}!${EOL.trim()}`;
