declare global {
    function print(message: string): void;
}

export function render() {
  print("Hello, VitaDeck!");
  print(`34 + 35 = ${34 + 35}`);
}
