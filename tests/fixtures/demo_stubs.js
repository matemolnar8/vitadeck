// Prepended to the demo Deck App by tests/run_smoke_test.sh so the clock, uptime and fetch pages render the same
// pixels on every run.
(() => {
  const RealDate = Date;
  const fixedTime = new RealDate(2026, 0, 2, 12, 34, 56).getTime();
  globalThis.Date = class extends RealDate {
    constructor(...args) {
      super(...(args.length > 0 ? args : [fixedTime]));
    }
    static now() {
      return fixedTime;
    }
  };
  globalThis.getTime = () => 42;
  const repo = {
    full_name: "matemolnar8/vitadeck",
    description: "React-authored Deck Apps on a PlayStation Vita",
    stargazers_count: 128,
    open_issues_count: 7,
  };
  globalThis.fetch = () =>
    Promise.resolve({
      ok: true,
      status: 200,
      statusText: "OK",
      headers: {},
      text: () => Promise.resolve(JSON.stringify(repo)),
      json: () => Promise.resolve(repo),
    });
})();
