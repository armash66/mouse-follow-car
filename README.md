# Mouse Follow Car

A lightweight 2D vehicle simulation built in C++ using SDL2.

This project starts from an empty window and builds a complete real-time mouse-guided car system from scratch.  
No game engine. No physics libraries. Only C++ and structured design.

## What This Project Does

This program creates a real-time simulation where a rectangular vehicle:

- Exists inside a render loop
- Maintains its own position, velocity, and acceleration
- Follows the mouse using vector mathematics
- Applies friction and speed limits
- Rotates based on movement direction
- Simulates basic drift behavior
- Runs efficiently with attention to CPU and memory usage

All movement and physics logic are implemented manually.

## Technical Focus

- C++
- SDL2
- Real-time update loop
- Vector-based movement
- Acceleration system
- Velocity clamping
- Drift mechanics
- Clean memory management
- Minimal runtime overhead

## Philosophy

This project is built step by step from zero:

Window → Render Loop → Object State → Movement → Physics → Optimization.

The goal is to understand how a real-time system works internally rather than relying on a game engine.

