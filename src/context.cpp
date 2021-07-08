#include "context.h"	
#include "image.h"
#include <imgui.h>

ContextUPtr Context::Create() {
    auto context = ContextUPtr(new Context());
    if (!context->Init())
        return nullptr;
    return std::move(context);
}

void Context::ProcessInput(GLFWwindow* window) {

    if (!m_cameraControl)
        return;

    const float cameraSpeed = 0.05f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        m_cameraPos += cameraSpeed * m_cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        m_cameraPos -= cameraSpeed * m_cameraFront;

    auto cameraRight = glm::normalize(glm::cross(m_cameraUp, -m_cameraFront));   
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        m_cameraPos += cameraSpeed * cameraRight;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        m_cameraPos -= cameraSpeed * cameraRight;    

    auto cameraUp = glm::normalize(glm::cross(-m_cameraFront, cameraRight));    
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        m_cameraPos += cameraSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        m_cameraPos -= cameraSpeed * cameraUp;
}

void Context::Reshape(int width, int height) {
    m_width = width;
    m_height = height;
    glViewport(0, 0, m_width, m_height);

    m_framebuffer = Framebuffer::Create(Texture::Create(width, height, GL_RGBA));
}

void Context::MouseMove(double x, double y) {
    if (!m_cameraControl)
        return;
    auto pos = glm::vec2((float)x, (float)y);
    auto deltaPos = pos - m_prevMousePos;
 
    const float cameraRotSpeed = 0.8f;
    m_cameraYaw -= deltaPos.x * cameraRotSpeed;
    m_cameraPitch -= deltaPos.y * cameraRotSpeed;

    if (m_cameraYaw < 0.0f)   m_cameraYaw += 360.0f;
    if (m_cameraYaw > 360.0f) m_cameraYaw -= 360.0f;

    if (m_cameraPitch > 89.0f)  m_cameraPitch = 89.0f;
    if (m_cameraPitch < -89.0f) m_cameraPitch = -89.0f;

    m_prevMousePos = pos;    
}

void Context::MouseButton(int button, int action, double x, double y) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            // 마우스 조작 시작 시점에 현재 마우스 커서 위치 저장
            m_prevMousePos = glm::vec2((float)x, (float)y);
            m_cameraControl = true;
        }
            else if (action == GLFW_RELEASE) {
            m_cameraControl = false;
        }
    }
}

bool Context::Init() {
    glEnable(GL_MULTISAMPLE);
    m_box = Mesh::CreateBox();
    m_plane = Mesh::CreatePlane();
    m_sphere = Mesh::CreateSphere();

    m_simpleProgram = Program::Create("./shader/simple.vs", "./shader/simple.fs");
    if (!m_simpleProgram)
        return false;

    m_program = Program::Create("./shader/lighting.vs", "./shader/lighting.fs");
    if (!m_program)
        return false;

    m_textureProgram = Program::Create("./shader/texture.vs", "./shader/texture.fs");
    if (!m_textureProgram)
        return false;

    m_postProgram = Program::Create("./shader/texture.vs","./shader/gamma.fs");
    if (!m_postProgram)
        return false;

    glClearColor(0.0f, 0.5f, 1.0f, 0.0f);
    
    TexturePtr darkGrayTexture = Texture::CreateFromImage(
        Image::CreateSingleColorImage(4, 4, glm::vec4(0.2f, 0.2f, 0.2f, 1.0f)).get());

    TexturePtr grayTexture = Texture::CreateFromImage(
        Image::CreateSingleColorImage(4, 4, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)).get());

    // 태양계 texture
    m_sunMaterial = Material::Create();
    m_sunMaterial->diffuse = Texture::CreateFromImage(Image::Load("./image/sun.jpg").get());
    m_sunMaterial->specular = grayTexture;
    m_sunMaterial->shininess = 64.0f;

    m_mercuryMaterial = Material::Create();
    m_mercuryMaterial->diffuse = Texture::CreateFromImage(Image::Load("./image/mercury.jpg").get());
    m_mercuryMaterial->specular = grayTexture;
    m_mercuryMaterial->shininess = 16.0f;

    m_venusMaterial = Material::Create();
    m_venusMaterial->diffuse = Texture::CreateFromImage(Image::Load("./image/venus.jpg").get());
    m_venusMaterial->specular = grayTexture;
    m_venusMaterial->shininess = 16.0f;

    m_earthMaterial = Material::Create();
    m_earthMaterial->diffuse = Texture::CreateFromImage(Image::Load("./image/earth.jpg").get());
    m_earthMaterial->specular = grayTexture;
    m_earthMaterial->shininess = 16.0f;

    m_moonMaterial = Material::Create();
    m_moonMaterial->diffuse = Texture::CreateFromImage(Image::Load("./image/moon.jpg").get());
    m_moonMaterial->specular = grayTexture;
    m_moonMaterial->shininess = 16.0f;

    m_marsMaterial = Material::Create();
    m_marsMaterial->diffuse = Texture::CreateFromImage(Image::Load("./image/mars.jpg").get());
    m_marsMaterial->specular = grayTexture;
    m_marsMaterial->shininess = 16.0f;

    auto cubeRight = Image::Load("./image/space/right.png", false);
    auto cubeLeft = Image::Load("./image/space/left.png", false);
    auto cubeTop = Image::Load("./image/space/top.png", false);
    auto cubeBottom = Image::Load("./image/space/bottom.png", false);
    auto cubeFront = Image::Load("./image/space/front.png", false);
    auto cubeBack = Image::Load("./image/space/back.png", false);
    m_cubeTexture = CubeTexture::CreateFromImages({
        cubeRight.get(),
        cubeLeft.get(),
        cubeTop.get(),
        cubeBottom.get(),
        cubeFront.get(),
        cubeBack.get(),
    });
    m_skyboxProgram = Program::Create("./shader/skybox.vs", "./shader/skybox.fs");
    m_envMapProgram = Program::Create("./shader/env_map.vs", "./shader/env_map.fs");

    m_shadowMap = ShadowMap::Create(1024, 1024);
    m_lightingShadowProgram = Program::Create("./shader/lighting_shadow.vs", "./shader/lighting_shadow.fs");

    return true;
}

void Context::Render() { 
    const char* s_planet[] = {"solarsystem","sun","mercury","venus","earth","moon","mars"};
    static int planet_current = 0;
    if (ImGui::Begin("UI Window")) {
        if(ImGui::ColorEdit4("Clear Color", glm::value_ptr(m_clearColor))){
            glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b,m_clearColor.a);
        }
        ImGui::DragFloat("gamma", &m_gamma, 0.01f, 0.0f, 2.0f);
        ImGui::Separator();
        ImGui::DragFloat3("Camera Pos", glm::value_ptr(m_cameraPos), 0.01f);
        ImGui::DragFloat("Camera Yaw", &m_cameraYaw, 0.5f);
        ImGui::DragFloat("Camera Pitch", &m_cameraPitch, 0.5f, -89.0f, 89.0f);
        ImGui::Separator();
        if (ImGui::Button("Reset Camera")) {
            m_cameraYaw = 0.0f;
            m_cameraPitch = -89.0f;
            m_cameraPos = glm::vec3(5.0f, 20.0f, 0.0f);
        } 	 	
        ImGui::Combo("SelectPlanet", &planet_current, s_planet, IM_ARRAYSIZE(s_planet));
        ImGui::Checkbox("rotating", &m_rotating);
        ImGui::Checkbox("revolution", &m_revolution);
    }
    ImGui::End(); 	

    auto lightView = glm::lookAt(m_light.position,
        m_light.position + m_light.direction,
        glm::vec3(0.0f, 1.0f, 0.0f));  	
 
    m_shadowMap->Bind();
    glClear(GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0,
        m_shadowMap->GetShadowMap()->GetWidth(),
        m_shadowMap->GetShadowMap()->GetHeight());
    m_simpleProgram->Use();
    m_simpleProgram->SetUniform("color", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    Framebuffer::BindToDefault();
    glViewport(0, 0, m_width, m_height);
 	
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
	
    m_cameraFront =
        glm::rotate(glm::mat4(1.0f), glm::radians(m_cameraYaw), glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), glm::radians(m_cameraPitch), glm::vec3(1.0f, 0.0f, 0.0f)) *
        glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);

    auto projection = glm::perspective(glm::radians(45.0f),
        (float)m_width / (float)m_height, 0.01f, 100.0f); //어디서 어디까지보여주는지 결정해주는것

    auto view = glm::lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);  

    //공전 
    const float pi = 3.141592f;
    float mercury_Rangle = (360.f / 180.0f * (float)glfwGetTime() * pi / 88.0f);
    float mercury_x = cos(mercury_Rangle) * 5.0f;
    float mercury_z = sin(mercury_Rangle) * 5.0f;
    float venus_Rangle = (360.f / 180.0f * (float)glfwGetTime() * pi / 225.0f);
    float venus_x = cos(venus_Rangle) * 7.0f;
    float venus_z = sin(venus_Rangle) * 7.0f;
    float earth_Rangle = (360.f / 180.0f * (float)glfwGetTime() * pi / 365.0f); 
    float earth_x = cos(earth_Rangle) * 9.0f;
    float earth_z = sin(earth_Rangle) * 9.0f;
    float mars_Rngle = (360.f / 180.0f * (float)glfwGetTime() * pi / 687.0f);
    float mars_x = cos(mars_Rngle) * 12.0f;
    float mars_z = sin(mars_Rngle) * 12.0f;
    float moon_Rangle = (360.f / 180.0f * (float)glfwGetTime() * pi / 27.0f);
    float moon_x = earth_x + cos(moon_Rangle) * 1.0f;
    float moon_z = earth_z + sin(moon_Rangle) * 1.0f;
    if(!m_revolution){
        mercury_x = 5.0f;
        mercury_z = 0.0f;
        venus_x = 7.0f;
        venus_z = 0.0f;
        earth_x = 9.0f;
        earth_z = 0.0f;
        mars_x = 12.0f;
        mars_z = 0.0f;
        moon_x = 10.0f;
        moon_z = 0.0f;
    }
    if(planet_current == 1){
        m_cameraPos = glm::vec3(0.0f, 15.0f, 0.0f);
        m_cameraPitch = -89.0f;
    }
    else if(planet_current == 2){
        m_cameraPos = glm::vec3(mercury_x + 0.5f, 5.0f, mercury_z + 0.5f);
        m_cameraYaw = 45.0f;
        m_cameraPitch = 0.0f;
    }
    else if(planet_current == 3){
        m_cameraPos = glm::vec3(venus_x + 1.0f, 5.0f, venus_z + 1.0f);
        m_cameraYaw = 45.0f;
        m_cameraPitch = 0.0f;
    }
    else if(planet_current == 4){
        m_cameraPos = glm::vec3(earth_x + 1.2f, 5.0f, earth_z + 1.2f);
        m_cameraYaw = 45.0f;
        m_cameraPitch = 0.0f;
    }
    else if(planet_current == 5){
        m_cameraPos = glm::vec3(moon_x - 0.2f, 5.0f, moon_z - 0.2f);
        m_cameraYaw = 225.0f;
        m_cameraPitch = 0.0f;
    }
    else if(planet_current == 6){
        m_cameraPos = glm::vec3(mars_x + 1.0f, 5.0f, mars_z + 1.0f);
        m_cameraYaw = 45.0f;
        m_cameraPitch = 0.0f;
    }

    	
    auto skyboxModelTransform =
        glm::translate(glm::mat4(1.0), m_cameraPos) *
        glm::scale(glm::mat4(1.0), glm::vec3(50.0f));
    m_skyboxProgram->Use();
    m_cubeTexture->Bind();
    m_skyboxProgram->SetUniform("skybox", 0);
    m_skyboxProgram->SetUniform("transform", projection * view * skyboxModelTransform);
    m_box->Draw(m_skyboxProgram.get());

    glm::vec3 lightPos = m_light.position;
    glm::vec3 lightDir = m_light.direction;

    auto lightModelTransform =
        glm::translate(glm::mat4(1.0), m_light.position) *
        glm::scale(glm::mat4(1.0), glm::vec3(0.1f)); 	
    m_simpleProgram->Use();
    m_simpleProgram->SetUniform("color", glm::vec4(m_light.ambient + m_light.diffuse, 1.0f));
    m_simpleProgram->SetUniform("transform", projection * view * lightModelTransform);
    m_sphere->Draw(m_simpleProgram.get());
    
    m_lightingShadowProgram->Use();
    m_lightingShadowProgram->SetUniform("viewPos", m_cameraPos);   	
    m_lightingShadowProgram->SetUniform("light.position", m_light.position);
    m_lightingShadowProgram->SetUniform("light.direction", m_light.direction);
    m_lightingShadowProgram->SetUniform("light.attenuation", GetAttenuationCoeff(m_light.distance));
    m_lightingShadowProgram->SetUniform("light.ambient", m_light.ambient);
    m_lightingShadowProgram->SetUniform("light.diffuse", m_light.diffuse);
    m_lightingShadowProgram->SetUniform("light.specular", m_light.specular);
    m_lightingShadowProgram->SetUniform("lightTransform", lightView);
    glActiveTexture(GL_TEXTURE3);
    m_shadowMap->GetShadowMap()->Bind();
    m_lightingShadowProgram->SetUniform("shadowMap", 3);
    glActiveTexture(GL_TEXTURE0);
    
    DrawScene(view, projection, m_lightingShadowProgram.get());
}

void Context::DrawScene(const glm::mat4& view,
    const glm::mat4& projection,
    const Program* program) {
    //자전  1S = 24H
    glm::mat4 sun_roatation = glm::rotate(glm::mat4(1.0f), glm::radians((float)glfwGetTime() * 14.4f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 mercury_roatation = glm::rotate(glm::mat4(1.0f), glm::radians((float)glfwGetTime() * 6.1f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 venus_roatation = glm::rotate(glm::mat4(1.0f), glm::radians((float)glfwGetTime() * -1.48f), glm::vec3(0.0f, 1.0f, 1.0f));
    glm::mat4 earth_roatation = glm::rotate(glm::mat4(1.0f), glm::radians((float)glfwGetTime() * 360.0f), glm::vec3(0.0f, 1.0f, 0.2f));
    glm::mat4 moon_roatation = glm::rotate(glm::mat4(1.0f), glm::radians((float)glfwGetTime() * 13.3f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 mars_roatation = glm::rotate(glm::mat4(1.0f), glm::radians((float)glfwGetTime() * 360.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    if(!m_rotating){
        sun_roatation = glm::rotate(glm::mat4(1.0f), glm::radians((float)glfwGetTime() * 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        mercury_roatation = glm::rotate(glm::mat4(1.0f), glm::radians((float)glfwGetTime() * 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        venus_roatation = glm::rotate(glm::mat4(1.0f), glm::radians((float)glfwGetTime() * 0.0f), glm::vec3(0.0f, 1.0f, 1.0f));
        earth_roatation = glm::rotate(glm::mat4(1.0f), glm::radians((float)glfwGetTime() * 0.0f), glm::vec3(0.0f, 1.0f, 0.2f));
        moon_roatation = glm::rotate(glm::mat4(1.0f), glm::radians((float)glfwGetTime() * 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        mars_roatation = glm::rotate(glm::mat4(1.0f), glm::radians((float)glfwGetTime() * 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    //공전 
    const float pi = 3.141592f;
    float mercury_Rangle = (360.f / 180.0f * (float)glfwGetTime() * pi / 88.0f);
    float mercury_x = cos(mercury_Rangle) * 5.0f;
    float mercury_z = sin(mercury_Rangle) * 5.0f;
    float venus_Rangle = (360.f / 180.0f * (float)glfwGetTime() * pi / 225.0f);
    float venus_x = cos(venus_Rangle) * 7.0f;
    float venus_z = sin(venus_Rangle) * 7.0f;
    float earth_Rangle = (360.f / 180.0f * (float)glfwGetTime() * pi / 365.0f); 
    float earth_x = cos(earth_Rangle) * 9.0f;
    float earth_z = sin(earth_Rangle) * 9.0f;
    float mars_Rngle = (360.f / 180.0f * (float)glfwGetTime() * pi / 687.0f);
    float mars_x = cos(mars_Rngle) * 12.0f;
    float mars_z = sin(mars_Rngle) * 12.0f;
    float moon_Rangle = (360.f / 180.0f * (float)glfwGetTime() * pi / 27.0f);
    float moon_x = earth_x + cos(moon_Rangle) * 1.0f;
    float moon_z = earth_z + sin(moon_Rangle) * 1.0f;
    if(!m_revolution){
        mercury_x = 5.0f;
        mercury_z = 0.0f;
        venus_x = 7.0f;
        venus_z = 0.0f;
        earth_x = 9.0f;
        earth_z = 0.0f;
        mars_x = 12.0f;
        mars_z = 0.0f;
        moon_x = 10.0f;
        moon_z = 0.0f;
    }
    glm::mat4 mercury_revoultion = glm::translate(glm::mat4(1.0f), glm::vec3(mercury_x, 5.0f, mercury_z));
    glm::mat4 venus_revoultion = glm::translate(glm::mat4(1.0f), glm::vec3(venus_x, 5.0f, venus_z));
    glm::mat4 earth_revoultion = glm::translate(glm::mat4(1.0f), glm::vec3(earth_x, 5.0f, earth_z));
    glm::mat4 mars_revoultion = glm::translate(glm::mat4(1.0f), glm::vec3(mars_x, 5.0f, mars_z));
    glm::mat4 moon_revoultion = glm::translate(glm::mat4(1.0f), glm::vec3(moon_x, 5.0f, moon_z));
    
    program->Use();
    //태양
    const int sphereCount = 7;
    const float offset = 1.2f;
    auto modelTransform =
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 5.0f, 0.0f))*
        sun_roatation *
        glm::scale(glm::mat4(1.0f), glm::vec3(5.0f, 5.0f, 5.0f));
        auto transform = projection * view * modelTransform;
        program->SetUniform("transform", transform);
        program->SetUniform("modelTransform", modelTransform);
        m_sunMaterial->SetToProgram(program);
        m_sphere->Draw(program);
    //수성
    modelTransform =
        mercury_revoultion*
        mercury_roatation *
        glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
        transform = projection * view * modelTransform;
        program->SetUniform("transform", transform);
        program->SetUniform("modelTransform", modelTransform);
        m_mercuryMaterial->SetToProgram(program);
        m_sphere->Draw(program);
    //화성
    modelTransform =
        venus_revoultion*
        venus_roatation *
        glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
        transform = projection * view * modelTransform;
        program->SetUniform("transform", transform);
        program->SetUniform("modelTransform", modelTransform);
        m_venusMaterial->SetToProgram(program);
        m_sphere->Draw(program);
    //지구
    modelTransform =
        earth_revoultion*
        earth_roatation *
        glm::scale(glm::mat4(1.0f), glm::vec3(1.2f, 1.2f, 1.2f));
        transform = projection * view * modelTransform;
        program->SetUniform("transform", transform);
        program->SetUniform("modelTransform", modelTransform);
        m_earthMaterial->SetToProgram(program);
        m_sphere->Draw(program);
    //달
    modelTransform =
        moon_revoultion*
        moon_roatation *
        glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f));
        transform = projection * view * modelTransform;
        program->SetUniform("transform", transform);
        program->SetUniform("modelTransform", modelTransform);
        m_moonMaterial->SetToProgram(program);
        m_sphere->Draw(program);
    //화성
    modelTransform =
        mars_revoultion*
        mars_roatation *
        glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 0.8f, 0.8f));
        transform = projection * view * modelTransform;
        program->SetUniform("transform", transform);
        program->SetUniform("modelTransform", modelTransform);
        m_marsMaterial->SetToProgram(program);
        m_sphere->Draw(program);
}
 