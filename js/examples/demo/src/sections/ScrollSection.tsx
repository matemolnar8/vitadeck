import { Button, Rect, Scroll, Stack, useTheme } from "@vitadeck/sdk";
import React, { useState } from "react";
import { CONTENT_WIDTH, Label, Section } from "../ui";

const ROWS = Array.from({ length: 40 }, (_, index) => index + 1);
const VIEWPORT_HEIGHT = 250;
const ROW_WIDTH = CONTENT_WIDTH - 24;

export function ScrollSection() {
  const { theme } = useTheme();
  const [picked, setPicked] = useState(0);

  return (
    <Section
      title="Scroll"
      blurb="A column Stack with a fixed viewport. Drag it, flick the left stick while a Button inside has Focus, or use the mouse wheel; it keeps gliding with momentum."
    >
      {/* A Stack does not lay out a Scroll, so a Rect of the same size holds its place. */}
      <Rect width={CONTENT_WIDTH} height={VIEWPORT_HEIGHT}>
        <Scroll x={0} y={0} width={CONTENT_WIDTH} height={VIEWPORT_HEIGHT} gap={6} padding={12} color={theme.surface}>
          <Button
            width={ROW_WIDTH}
            height={40}
            label={
              picked === 0
                ? "Press to focus the list, then use the left stick"
                : `Pressed ${picked}× · focus is in the list`
            }
            color={theme.primary}
            textColor={theme.buttonText}
            fontSize={16}
            borderRadius={6}
            onPress={() => {
              setPicked((count) => count + 1);
            }}
          />
          {ROWS.map((row) => (
            <Stack
              key={row}
              direction="row"
              width={ROW_WIDTH}
              height={32}
              padding={6}
              gap={10}
              align="center"
              color={row % 2 === 0 ? theme.surfaceAlt : theme.background}
            >
              <Rect width={20} height={20} color={row % 5 === 0 ? theme.accent : theme.outline} borderRadius={10} />
              <Label width={ROW_WIDTH - 50}>{`Row ${row} of ${ROWS.length}`}</Label>
            </Stack>
          ))}
        </Scroll>
      </Rect>
    </Section>
  );
}
