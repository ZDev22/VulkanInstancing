#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) flat out uint textureId;

layout(set = 0, binding = 0) readonly buffer SpriteData {
    vec2 translation;
    mat2 transform;
    vec3 color;
    float padding;
    uint textureId;
    vec2 speed;
} sprites[];

layout(push_constant) uniform Push {
    mat4 projection;
} push;

void main() {
    uint instanceIndex = gl_InstanceIndex;
    vec2 pos = sprites[instanceIndex].transform * inPosition;
    pos += sprites[instanceIndex].translation;
    gl_Position = push.projection * vec4(pos, 0.0, 1.0);
    fragColor = sprites[instanceIndex].color;
    fragTexCoord = inTexCoord;
    textureId = sprites[instanceIndex].textureId;
}