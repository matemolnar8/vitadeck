let intervals = new Map<number, number>();
let intervalIdCounter = 0;

globalThis.setInterval = (callback: () => void, delay: number) => {
  const intervalId = intervalIdCounter++;

  const interval = () => {
    callback();
    const newTimeoutId = setTimeout(interval, delay);
    intervals.set(intervalId, newTimeoutId);
  };

  const timeoutId = setTimeout(interval, delay);
  intervals.set(intervalId, timeoutId);

  return intervalId;
};

globalThis.clearInterval = (intervalId: number) => {
  if (!intervals.has(intervalId)) {
    console.error("Interval not found");
    return;
  }
  clearTimeout(intervals.get(intervalId)!);
  intervals.delete(intervalId);
};
