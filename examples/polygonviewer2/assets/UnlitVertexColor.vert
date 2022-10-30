#version 300 es

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec4 inColor;

uniform float scale;

out vec4 fragColor;

void main() {
  vec2 newPosition = inPosition * scale;
  gl_Position = vec4(newPosition, 0, 1);
  fragColor = inColor;
}