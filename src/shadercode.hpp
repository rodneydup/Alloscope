// the following is the shader code, which allows for the fuzzy look

namespace shader {
const char *vertexCode = R"(
#version 400

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec4 vertexColor;
layout(location = 2) in vec2
    vertexThickness;  // as a 2D texture cordinate, but we ignore the y

out Vertex {
  vec4 color;
  float size;
}
vertex;

void main() {
  gl_Position = vec4(vertexPosition, 1.0);
  vertex.color = vertexColor;
  vertex.size = vertexThickness.x;
}
)";

const char *geometryCode = R"(
#version 400

// take in a point and output a triangle strip with 4 vertices (aka a "quad")
//
layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

in Vertex {
  vec4 color;
  float size;
}
vertex[];

out Fragment {
  vec4 color;
  float textureCoordinate;
}
fragment;

void main() {
  vec4 a = gl_in[0].gl_Position;
  vec4 b = gl_in[1].gl_Position;

  const float r = 0.03;

  vec4 d = vec4(normalize(cross(b.xyz - a.xyz, vec3(0.0, 0.0, 1.0))), 0.0) * r;

  gl_Position = a + d * vertex[0].size;
  fragment.color = vertex[0].color;
  fragment.textureCoordinate = 0.0;
  EmitVertex();

  gl_Position = a - d * vertex[0].size;
  fragment.color = vertex[0].color;
  fragment.textureCoordinate = 1.0;
  EmitVertex();

  gl_Position = b + d * vertex[1].size;
  fragment.color = vertex[1].color;
  fragment.textureCoordinate = 0.0;
  EmitVertex();

  gl_Position = b - d * vertex[1].size;
  fragment.color = vertex[1].color;
  fragment.textureCoordinate = 1.0;
  EmitVertex();

  EndPrimitive();
}
)";

const char *fragmentCode = R"(
#version 400

in Fragment {
  vec4 color;
  float textureCoordinate;
}
fragment;

uniform sampler1D alphaTexture;

layout(location = 0) out vec4 fragmentColor;

void main() {
  float a = texture(alphaTexture, fragment.textureCoordinate).r;
  if (a < 0.05) discard;
  fragmentColor = vec4(fragment.color.xyz, fragment.color.a * a);
}
)";
}  // namespace shader