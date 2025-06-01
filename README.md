# Ray Tracing Techniques

This repository contains several fundamental ray tracing techniques implemented in C++. The techniques, ranging from basic ray tracing to advanced methods like global illumination and volume rendering.

## Implemented Techniques

- **Ray Tracing** with Blinn-Phong shading for rendering 3D meshes.
- **Whitted Ray Tracing** with support for reflection and refraction.
- **Path Tracing** (Global Illumination) using BRDF sampling and Next Event Estimation.
- **Volume Rendering** via both Forward and Backward Ray Marching.

## Results

### Ray Tracing with Blinn-Phong Shading
<p align="center">
  <img src="results/sphere.png" width="75%" alt="Sphere"/>
</p>
<p align="center"><i>Sphere on the ground</i></p>
<p align="center">
  <img src="results/barrel.png" width="45%" alt="Barrel"/>
  <img src="results/motocompo.png" width="45%" alt="Motocompo"/>
</p>
<p align="center"><i>Left: Barrel | Right: Motocompo</i></p>

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