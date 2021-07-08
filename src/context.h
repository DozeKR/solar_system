#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include "common.h"
#include "shader.h"
#include "program.h"
#include "buffer.h"
#include "vertex_layout.h"
#include "texture.h"
#include "mesh.h"
#include "model.h"
#include "framebuffer.h"
#include "shadow_map.h"

CLASS_PTR(Context)
class Context {
public:
    static ContextUPtr Create();
    void Render(); 
    void ProcessInput(GLFWwindow* window);
    void Reshape(int width, int height);
    void MouseMove(double x, double y);
    void MouseButton(int button, int action, double x, double y);

    void DrawScene(const glm::mat4& view,
        const glm::mat4& projection,
        const Program* program);

private:
    Context() {}
    bool Init();
    ProgramUPtr m_program;
    ProgramUPtr m_simpleProgram;
    ProgramUPtr m_textureProgram;
    ProgramUPtr m_postProgram;
    float m_gamma {1.0f};

    MeshUPtr m_box;	
    MeshUPtr m_plane;
    MeshUPtr m_sphere;
  
    // animation
    bool m_revolution { true };
    bool m_rotating { true };
    glm::vec3 m_rotspeed { glm::vec3(0.0f, 0.0f, 0.0f) };
    glm::vec3 m_rotation { glm::vec3(0.0f, 10.0f, 0.0f) };

    // clear color
    glm::vec4 m_clearColor { glm::vec4(0.0f, 0.0f, 0.3f, 0.0f) };

    // light parameter
    struct Light {
        glm::vec3 position { glm::vec3(0.0f, 5.0f, 0.0f) };
        glm::vec3 direction { glm::vec3(-0.5f, -1.5f, -1.0f) };
        float distance { 150.0f };
        glm::vec3 ambient { glm::vec3(1.0f, 1.0f, 1.0f) };
        glm::vec3 diffuse { glm::vec3(1.0f, 1.0f, 1.0f) };
        glm::vec3 specular { glm::vec3(1.0f, 1.0f, 1.0f) };
    };
    Light m_light;
    bool m_flashLightMode {false};
    
    // material parameter
    MaterialPtr m_planeMaterial;
    MaterialPtr m_box1Material;
    MaterialPtr m_box2Material;
    MaterialPtr m_sunMaterial;
    MaterialPtr m_mercuryMaterial;
    MaterialPtr m_venusMaterial;
    MaterialPtr m_earthMaterial;
    MaterialPtr m_moonMaterial;
    MaterialPtr m_marsMaterial;
    // 
    TexturePtr m_windowTexture;

    // camera parameter
    bool m_cameraControl { false };
    glm::vec2 m_prevMousePos { glm::vec2(0.0f) };
    float m_cameraPitch { 0.0f };                             //피치각
    float m_cameraYaw { 45.0f };                                 //요각
    glm::vec3 m_cameraPos { glm::vec3(15.0f, 6.0f, 7.0f) };      //카메라 위치
    glm::vec3 m_cameraFront { glm::vec3(0.0f, 0.0f, -1.0f) };   //카메라 바라보는 방향 
    glm::vec3 m_cameraUp { glm::vec3(0.0f, 1.0f, 0.0f) };       //카메라 화면의 세로 축 방향

    // framebuffer
    FramebufferUPtr m_framebuffer;

    // cubemap
    CubeTextureUPtr m_cubeTexture;
    ProgramUPtr m_skyboxProgram;
    ProgramUPtr m_envMapProgram;
    
    // shadow map
    ShadowMapUPtr m_shadowMap;
    ProgramUPtr m_lightingShadowProgram;

    int m_width {WINDOW_WIDTH};
    int m_height {WINDOW_HEIGHT};
};

#endif // __CONTEXT_H__