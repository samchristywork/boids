![Banner](https://s-christy.com/status-banner-service/boids/banner-slim.svg)

## Overview

The Boids algorithm is a behavior model which simulates flocking behavior found
in the wild exhibited by animals like birds and fish. This implementation is
written in C using the SDL2 graphics library. I have been able to get decent
performance using quadtrees.

Quadtrees are a data structure that can efficiently store and retrieve objects
in a 2D space. By dividing the space into smaller quadrants recursively,
quadtrees can reduce the number of objects that need to be checked for collision
detection or proximity testing, thus improving the performance of the swarming
algorithm.

I have added custom widgets to the interface which allow the users to customize
the various parameters of the swarming algorithm, such as speed, separation,
alignment, and cohesion. The software also has different view modes that allows
the user to zoom in, follow a particular boid, and view the vectors generated by
the various rules in a debug mode. The debug mode also shows how the quadtree is
structured.

## Screenshots

<p align="center">
  <img src="./res/screenshot.png" />
</p>

## Features

- Simulation of the Boids swarming algorithm
- Uses the SDL2 graphics library
- Uses quadtrees to improve performance
- Customizable parameters, such as speed, separation, alignment, and cohesion
- Different view modes available, including zoom, follow, and debug
- Debug mode shows vectors generated by various rules and quadtree structure
- Fullscreen and windowed modes
- Configurable FPS target to speed up or slow down the simulation
- Option to dynamically add and remove boids to hit FPS targets
- Parsing of command line arguments

## Usage

```
Usage: ./main
  -c,--no-cap-framerate  Start with a uncapped framerate.
  -d,--debug             Start with debug view enabled.
  -f,--fps               Target FPS (default 60).
  -h,--help              Display Usage statement.
  -n,--num               Number of boids in simulation (default 256).
  -p,--pause             Start paused.
  -s,--seed              Seed to use for random generation.
  -u,--fullscreen        Fullscreen mode.
  -y,--dynamic           Number of boids dynamically changes based on framerate.
```

## Dependencies

```
gcc
libsdl2-dev
libsdl2-gfx-dev
libsdl2-ttf-dev
make
```

## License

This work is licensed under the GNU General Public License version 3 (GPLv3).

[<img src="https://s-christy.com/status-banner-service/GPLv3_Logo.svg" width="150" />](https://www.gnu.org/licenses/gpl-3.0.en.html)
