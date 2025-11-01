import React, { useState } from "react";
import { Counter } from "../components/Counter";
import { useTheme } from "../theme";

export const Hello = () => {
  const { theme } = useTheme();
  const [clickedButton, setClickedButton] = useState<string | null>(null);

  const handleBottomClick = () => {
    setClickedButton("bottom");
  };

  const handleTopClick = () => {
    setClickedButton("top");
  };

  return (
    <>
      <vita-rect x={20} y={20} width={920} height={80} color={theme.surface}>
        <vita-text fontSize={28} color={theme.text}>
          Hello, world!
        </vita-text>
      </vita-rect>

      <Counter x={30} y={130} />

      {/* Overlapping buttons test */}
      <vita-rect x={20} y={240} width={920} height={200} color={theme.surface}>
        <vita-text fontSize={22} color={theme.text}>
          Overlapping Buttons Test
        </vita-text>
        <vita-text fontSize={18} color={theme.text}>
          Click the overlapping area to test which button handles the click
        </vita-text>
        <vita-text fontSize={18} color={theme.text}>
          {clickedButton
            ? `Last clicked: ${clickedButton} button`
            : "No button clicked yet"}
        </vita-text>

        {/* Bottom button (larger, rendered first) */}
        <vita-button
          x={50}
          y={100}
          width={300}
          height={60}
          color={Colors.RED}
          label="Bottom Button"
          onClick={handleBottomClick}
        />

        {/* Top button (smaller, rendered second, should be on top) */}
        <vita-button
          x={200}
          y={120}
          width={300}
          height={60}
          color={Colors.GREEN}
          label="Top Button"
          onClick={handleTopClick}
        />
      </vita-rect>
    </>
  );
};
