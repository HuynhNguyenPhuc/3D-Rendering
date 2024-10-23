from PIL import Image

def convert(input_file, output_file):
    try:
        with Image.open(input_file) as img:
            img.save(output_file)
        print(f"Image saved as {output_file}")
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    input_files = ["results/whitted_ray_tracing.ppm", "results/laurel.ppm", "results/volume_rendering.ppm", "results/forward_ray_marching.ppm", "results/backward_ray_marching.ppm"]
    output_files = ["results/whitted_ray_tracing.png", "results/laurel.png", "results/volume_rendering.png","results/forward_ray_marching.png", "results/backward_ray_marching.png"]
    
    for input_file, output_file in zip(input_files, output_files):
        convert(input_file, output_file)