#version 330 core
out vec4 fragColor;
in vec2 texCoord;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;	
uniform sampler2D ssao;
uniform int useSsao;

struct Light {
    vec3 position;
    vec3 color;
};
const int NR_LIGHTS = 32;
uniform Light lights[NR_LIGHTS];
uniform vec3 viewPos;
void main() {
    // retrieve data from G-buffer
    vec3 fragPos = texture(gPosition, texCoord).rgb;
    vec3 normal = texture(gNormal, texCoord).rgb;
    vec3 albedo = texture(gAlbedoSpec, texCoord).rgb;
    float specular = texture(gAlbedoSpec, texCoord).a;
    // then calculate lighting as usual  	
    vec3 ambient = useSsao == 1 ?
        texture(ssao, texCoord).r * 0.4 * albedo :
        albedo * 0.4; // hard-coded ambient component
    vec3 lighting = ambient; 
    
    vec3 viewDir = normalize(viewPos - fragPos);
    for(int i = 0; i < NR_LIGHTS; ++i) {
        // diffuse
        vec3 lightDir = normalize(lights[i].position - fragPos);
        vec3 diffuse = max(dot(normal, lightDir), 0.0) * albedo * lights[i].color;
        lighting += diffuse;

        //vec3 halfDir = moramlize(lightDir + viewDir);
        //float spec = pow(max(dot(helfDir, normal), 0.0), 32.0);
        //lighting += specualr * spec;
    }
    fragColor = vec4(lighting, 1.0);
}