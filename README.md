# Voronoi-Rasterizer

![Voronoi Output Showcase](images/interesting.gif)

![Voronoi Output Showcase](images/voronoi_4.png)
> Generated Voronoi Diagram using Euclidean Distance & Cyberpunk Palette

![Voronoi Output Showcase](images/voronoi_manhattan.png)
> Generated Voronoi Diagram using Manhattan Distance & Cyberpunk Palette

A lightweight, C-based tool to generate and visualize Voronoi diagrams using various distance metrics. It renders the output to `.ppm` image files or generates a time-lapse video (`.mp4`).

## Features

* Multiple Distance Metrics:
    * Euclidean: Standard Voronoi diagram using straight-line distance.
    * Manhattan: Voronoi diagram using taxicab geometry (diamond shapes).
* Procedural Generation:
    * Interesting Mode: A sequential visualization showing how the diagram evolves as seeds are added one by one.
* Output Formats:
    * Static Image (`.ppm`)
    * Video Animation (`.mp4`)
* No External Dependencies: Pure C implementation (except for ffmpeg if video export is needed).
* Custom Palette: Uses a carefully selected Cyberpunk color scheme.

## Build & Run

### Prerequisites
You need a C compiler (GCC or Clang). For MP4 export, `ffmpeg` is required.

### Compilation
You can use the provided build script:
```bash
f./build.sh
```

or compile manually:
```bash
gcc main.c -o voronoi
```

### Usage
The program accepts Render Mode and Save Mode as arguments.
```bash
./voronoi <render_mode> <save_mode>
```

Arguments:
* <render_mode>: euclidean, manhattan, interesting
* <save_mode>: ppm, mp4

### Example
Generate a standard Voronoi diagram as an image:
```bash
./voronoi euclidean ppm
```

Generate a Manhattan distance animation:
```bash
./voronoi manhattan mp4
```

## Future Work & Roadmap

This project is currently based on a pixel-by-pixel distance check (Brute-Force). The next major version aims to implement high-performance computational geometry algorithms:

* Advanced Algorithms: Implementation of Fortune's Algorithm (Sweep Line) to reduce complexity from O(W * H * N) to O(N log(N)), enabling real-time generation with thousands of seeds.
* Spatial Indexing: Integration of Quadtrees or k-d trees for faster nearest-neighbor queries in CPU mode.


### Acknowledgements

https://www.youtube.com/watch?v=kT-Mz87-HcQ&list=PLOI2XyQTagtKbei7ZTXbVq6jBy00ZpMep

https://youtu.be/qIOAz7M1syw?si=E440xV7CURtj_RL6

https://www.youtube.com/watch?v=xNX9H_ZkfNE

https://colormagic.app/palette/explore/cyberpunk