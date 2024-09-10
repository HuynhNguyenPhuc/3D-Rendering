#version 330 core
out vec4 FragColor;

in vec3 w_pos;
in vec3 normal;

uniform vec3 light_pos;
uniform vec3 view_pos;

vec3 calculate_irradiance(vec3 normal) {
    vec3 irradiance = vec3(0.0);
    return irradiance;
}

void main() {
    vec3 norm = normalize(normal);
    vec3 irradiance = calculate_irradiance(norm);

    FragColor = vec4(irradiance, 1.0);
}