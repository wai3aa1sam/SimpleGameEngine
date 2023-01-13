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

#  multi-threaded version boids demo
- for 10000 boids objects
- single-threaded update loop : 18.35ms (~54.5fps)
- multi-threaded update-loop  : 2.96ms  (~337.8fps, x6.19 speedup)
- capable of updating 30000 objects in the Boids loop in 9.1ms (109fps)
- warning: single-threaded version are not cache friendly, the result may have some bias.

![image](https://user-images.githubusercontent.com/120044193/210845309-b2b23266-132a-4dc2-9b1f-292cf629bc9d.png)

![image](https://user-images.githubusercontent.com/120044193/210845326-6cbb5777-920d-4e89-a9f2-38e9a65a8d53.png)

- delay 1 frame with 30000 boids object 
![30000-boids](https://user-images.githubusercontent.com/120044193/212269586-9a200c7f-1f0e-442e-80fb-9c6c6af28c2a.jpg)

## How to Build 

### Build on Windows
```
install_vcpkg_packages.bat
gen_vs2022.bat
```
- Open Visual Studio project in `build/Sam-SimpleGameEngine-x64-windows`
