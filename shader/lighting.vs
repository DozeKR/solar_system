#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 transform;
uniform mat4 modelTransform;

out vec3 normal;
out vec2 texCoord;
out vec3 position;

void main() {
    gl_Position = transform * vec4(aPos, 1.0);
    normal = (transpose(inverse(modelTransform)) * vec4(aNormal, 0.0)).xyz; //inverse transpose적용 이유: 점이 아닌 벡터는 이를 사용해야한다.
    texCoord = aTexCoord;
    position = (modelTransform * vec4(aPos, 1.0)).xyz;  //world space 상에서의 좌표값이 필요해서 사용
}