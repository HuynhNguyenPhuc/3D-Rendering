#version 330 core
out vec4 FragColor;

in vec3 w_pos;
in vec3 normal;

uniform vec3 light_pos;
uniform vec3 view_pos;

vec3 calculate_irradiance(vec3 normal) {
    vec3 irradiance = vec3(0.0);
    int numSamples = 16;
    float sample_weight = 1.0 / float(numSamples);

    for (int i = 0; i < numSamples; ++i) {
        float theta = float(i) * (3.14159 / float(numSamples));
        float phi = float(i) * (2.0 * 3.14159 / float(numSamples));

        vec3 sample_dir = vec3(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi));
        float cos_theta = max(dot(normal, sample_dir), 0.0);
        irradiance += sample_weight * cos_theta * vec3(0.0, 1.0, 0.0);
    }

    return irradiance;
}

void main() {
    vec3 norm = normalize(normal);
    vec3 irradiance = calculate_irradiance(norm);

    FragColor = vec4(irradiance, 1.0);
}