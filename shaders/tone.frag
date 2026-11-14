#version 450
#extension GL_EXT_debug_printf : enable

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D uSceneColor;

vec3 ACESFilm(vec3 x) {
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
    vec2 uv = vec2(vUV.x, 1.0 - vUV.y);
    // vec2 uv = vUV;
    vec3 color = texture(uSceneColor, uv).rgb;
    // color = color / (1 + color);
    color = ACESFilm(color);
    outColor = vec4(color.x, color.y, color.z, 1.0); // passthrough
    // if (ivec2(gl_FragCoord.xy) == ivec2(400, 300)) {
    //     debugPrintfEXT("uv = %v2f, color = %v3f", uv, color);
    // }


    // outColor = vec4(texelFetch(uSceneColor, ivec2(gl_FragCoord.xy), 0).rgb, 1.0);
    // texelFetch(uSceneColor, ivec2(gl_FragCoord.xy), 0);
}
