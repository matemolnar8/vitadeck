import React, { useEffect, useMemo, useState } from "react";

export const App = () => {
  const [updates, setUpdates] = useState(0);

  useEffect(() => {
    setUpdates((updates) => updates + 1);
  });

  const fizzBuzz = useMemo(() => {
    return updates % 15 === 0
      ? "FizzBuzz"
      : updates % 3 === 0
      ? "Fizz"
      : updates % 5 === 0
      ? "Buzz"
      : null;
  }, [updates]);

  return (
    <>
      <vita-text>Hello, VitaDeck! Updates: {updates}</vita-text>
      {fizzBuzz && <vita-text>{fizzBuzz}</vita-text>}
    </>
  );
};
