# SimpleGameEngine
- cloned from https://github.com/SimpleTalkCpp/SimpleGameEngine with extra features

## Features
- [Boids](#Boids)
- [Continuous LOD Terrain](#Continous_LOD_Terrain)
- [JobSystem](#JobSystem)

## Boids
- 10000 objects in scene with ~60fps when separation is on, ~35fps in default
- space partitioning

[boids_exmaple-part1.webm](https://user-images.githubusercontent.com/120044193/210183978-33beae4c-79cc-41cf-8785-96c090027694.webm)

[boids_exmaple-part2.webm](https://user-images.githubusercontent.com/120044193/210184017-12e8366f-ec91-480c-8722-863866563d79.webm)

### references
- https://www.youtube.com/watch?v=hLJVm89DULU
- https://www.youtube.com/watch?v=bqtqltqcQhw

## Continous_LOD_Terrain
- algorithm to generate same lod indices around adjacent patch to avoid T-Junctions

[clod_terrain_example.webm](https://user-images.githubusercontent.com/120044193/210183312-bc0f689b-7672-46dd-942c-b08155e132a4.webm)

### references
- https://www.cs.princeton.edu/courses/archive/spr01/cs598b/papers/lindstrom96.pdf

## JobSystem
- There is no demo in the current stage.
- Trying to use the Job System with boids to achieve better optimization.
- Coming soon.

## How to Build 

### Build on Windows
```
install_vcpkg_packages.bat
gen_vs2022.bat
```
- Open Visual Studio project in `build/Sam-SimpleGameEngine-x64-windows`
