#version 330 core
in vec4 vertexColor;
in vec2 texCoord;
out vec4 fragColor;

//동시에 쓸수 있는 텍스처 개수 는 32개까지 제한
uniform sampler2D tex;

void main() {
    vec4 pixel = texture(tex, texCoord);
    if (pixel.a < 0.01)
        discard;
    fragColor = pixel;
}
