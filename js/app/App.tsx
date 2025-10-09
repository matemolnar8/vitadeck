import React, { useCallback, useEffect, useState } from "react";

export const App = () => {
  const [count, setCount] = useState(0);

  const doTimeout = useCallback(() => {
    setTimeout(() => {
      setCount((count) => count + 1);
      doTimeout();
    }, 1000);
  }, []);

  useEffect(() => {
    doTimeout();
  }, []);

  return (
    <>
      <vita-text>Hello, VitaDeck! Updates: {count}</vita-text>
    </>
  );
};
