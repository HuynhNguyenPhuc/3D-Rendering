# Rendering Techniques

This repository contains several fundamental rendering techniques implemented in C++. The techniques, ranging from basic ray tracing to advanced methods like global illumination and volume rendering.

## Implemented Techniques

- **Ray Tracing** with Blinn-Phong shading for rendering 3D meshes.
- **Whitted Ray Tracing** with support for reflection and refraction.
- **Path Tracing** (Global Illumination) using BRDF sampling and Next Event Estimation.
- **Volume Rendering** via both Forward and Backward Ray Marching.

## Results

### Ray Tracing with Blinn-Phong Shading  
![Ray Tracing with Blinn Phong Shading](results/rendering.png)

### Whitted Ray Tracing 
![Whitted Ray Tracing with Reflection and Refraction](results/whitted_ray_tracing.png)

### Path Tracing
![Path Tracing with BRDF Sampling + Next Event Estimation](results/path_tracing.png)

### Volume Rendering: Forward vs Backward Ray Marching  
<p align="center">
  <img src="results/forward_ray_marching.png" width="45%" alt="Forward Ray Marching"/>
  <img src="results/backward_ray_marching.png" width="45%" alt="Backward Ray Marching"/>
</p>
<p align="center"><i>Left: Forward Ray Marching | Right: Backward Ray Marching</i></p>