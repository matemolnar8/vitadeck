import React, { useCallback, useEffect, useRef, useState } from "react";

type Vec2 = { x: number; y: number };

// Arena (coordinates inside this component are RELATIVE to the playfield rect)
const playfield = { x: 20, y: 20, w: 920, h: 400 };
const paddle = { w: 120, h: 16, speed: 10 };
const ballSize = 12;

export const GamePong = () => {
  // Player input flags (avoid re-renders)
  const pressedRef = useRef({ left: false, right: false });

  // Mutable game state refs (physics)
  // All positions below are RELATIVE to the playfield (0..w, 0..h)
  const playerXRef = useRef(playfield.w / 2 - paddle.w / 2);
  const aiXRef = useRef(playfield.w / 2 - paddle.w / 2);
  const ballPosRef = useRef<Vec2>({ x: playfield.w / 2, y: playfield.h / 2 });
  const ballVelRef = useRef<Vec2>({ x: 6, y: 6 });
  const scoreRef = useRef({ player: 0, ai: 0 });

  // Rendered snapshots
  const [playerX, setPlayerX] = useState(playerXRef.current);
  const [aiX, setAiX] = useState(aiXRef.current);
  const [ball, setBall] = useState(ballPosRef.current);
  const [score, setScore] = useState(scoreRef.current);

  const resetBall = useCallback((towardsPlayer: boolean) => {
    ballPosRef.current = { x: playfield.w / 2, y: playfield.h / 2 };
    ballVelRef.current = {
      x: (Math.random() < 0.5 ? -1 : 1) * 6,
      y: (towardsPlayer ? 1 : -1) * 6,
    };
  }, []);

  useEffect(() => {
    const id = setInterval(() => {
      // Input → player paddle
      if (pressedRef.current.left) playerXRef.current = Math.max(0, playerXRef.current - paddle.speed);
      if (pressedRef.current.right)
        playerXRef.current = Math.min(playfield.w - paddle.w, playerXRef.current + paddle.speed);

      // Simple AI follows ball with clamp
      const aiTarget = ballPosRef.current.x - paddle.w / 2;
      const aiSpeed = 8;
      if (aiXRef.current + 2 < aiTarget) aiXRef.current = Math.min(aiTarget, aiXRef.current + aiSpeed);
      else if (aiXRef.current - 2 > aiTarget) aiXRef.current = Math.max(aiTarget, aiXRef.current - aiSpeed);
      aiXRef.current = Math.max(0, Math.min(playfield.w - paddle.w, aiXRef.current));

      // Move ball
      const next: Vec2 = {
        x: ballPosRef.current.x + ballVelRef.current.x,
        y: ballPosRef.current.y + ballVelRef.current.y,
      };

      // Walls (left/right)
      if (next.x <= ballSize / 2 || next.x >= playfield.w - ballSize / 2) {
        ballVelRef.current.x *= -1;
      }

      // Top paddle (AI)
      const aiTop = 20;
      if (
        next.y - ballSize / 2 <= aiTop + paddle.h &&
        next.y - ballSize / 2 >= aiTop &&
        next.x >= aiXRef.current &&
        next.x <= aiXRef.current + paddle.w &&
        ballVelRef.current.y < 0
      ) {
        ballVelRef.current.y *= -1;
      }

      // Bottom paddle (Player)
      const playerY = playfield.h - 20 - paddle.h;
      if (
        next.y + ballSize / 2 >= playerY &&
        next.y + ballSize / 2 <= playerY + paddle.h &&
        next.x >= playerXRef.current &&
        next.x <= playerXRef.current + paddle.w &&
        ballVelRef.current.y > 0
      ) {
        ballVelRef.current.y *= -1;
      }

      // Scoring
      if (next.y < 0) {
        // Player scores
        scoreRef.current = {
          player: scoreRef.current.player + 1,
          ai: scoreRef.current.ai,
        };
        resetBall(true);
      } else if (next.y > playfield.h) {
        // AI scores
        scoreRef.current = {
          player: scoreRef.current.player,
          ai: scoreRef.current.ai + 1,
        };
        resetBall(false);
      } else {
        ballPosRef.current = next;
      }

      // Publish snapshots to render at modest rate
      setPlayerX(playerXRef.current);
      setAiX(aiXRef.current);
      setBall(ballPosRef.current);
      setScore(scoreRef.current);
    }, 33);
    return () => clearInterval(id);
  }, [resetBall]);

  const onLeftDown = useCallback(() => {
    pressedRef.current.left = true;
  }, []);
  const onLeftUp = useCallback(() => {
    pressedRef.current.left = false;
  }, []);
  const onRightDown = useCallback(() => {
    pressedRef.current.right = true;
  }, []);
  const onRightUp = useCallback(() => {
    pressedRef.current.right = false;
  }, []);

  const playerY = playfield.h - 20 - paddle.h; // relative to playfield
  const aiY = 20; // relative to playfield

  return (
    <>
      {/* Playfield */}
      <vita-rect x={playfield.x} y={playfield.y} width={playfield.w} height={playfield.h} color={Colors.DARKGRAY}>
        {/* Vertical center line dividing left/right sides */}
        <vita-rect x={0} y={playfield.h / 2 - 1} width={playfield.w} height={2} color={Colors.GRAY} />

        {/* Scores */}
        <vita-text fontSize={24} color={Colors.RAYWHITE}>
          Player {score.player} : {score.ai} AI
        </vita-text>

        {/* AI Paddle */}
        <vita-rect x={aiX} y={aiY} width={paddle.w} height={paddle.h} color={Colors.BLUE} />

        {/* Player Paddle */}
        <vita-rect x={playerX} y={playerY} width={paddle.w} height={paddle.h} color={Colors.GREEN} />

        {/* Ball */}
        <vita-rect
          x={ball.x - ballSize / 2}
          y={ball.y - ballSize / 2}
          width={ballSize}
          height={ballSize}
          color={Colors.WHITE}
        />
      </vita-rect>

      {/* Controls */}
      <vita-button
        x={20}
        y={playfield.y + playfield.h + 10}
        width={200}
        height={40}
        label={"Left"}
        onMouseDown={onLeftDown}
        onMouseUp={onLeftUp}
        onMouseLeave={onLeftUp}
      />
      <vita-button
        x={230}
        y={playfield.y + playfield.h + 10}
        width={200}
        height={40}
        label={"Right"}
        onMouseDown={onRightDown}
        onMouseUp={onRightUp}
        onMouseLeave={onRightUp}
      />
    </>
  );
};
