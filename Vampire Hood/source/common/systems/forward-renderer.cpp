#include "forward-renderer.hpp"
#include "../mesh/mesh-utils.hpp"
#include "../texture/texture-utils.hpp"
#include <iostream>
#include<string>
using namespace std;
namespace our {

    void ForwardRenderer::initialize(glm::ivec2 windowSize, const nlohmann::json& config){
        // First, we store the window size for later use
        this->windowSize = windowSize;

        // Then we check if there is a sky texture in the configuration
        if(config.contains("sky")){
            // First, we create a sphere which will be used to draw the sky
            this->skySphere = mesh_utils::sphere(glm::ivec2(16, 16));
            
            // We can draw the sky using the same shader used to draw textured objects
            ShaderProgram* skyShader = new ShaderProgram();
            skyShader->attach("assets/shaders/textured.vert", GL_VERTEX_SHADER);
            skyShader->attach("assets/shaders/textured.frag", GL_FRAGMENT_SHADER);
            skyShader->link();
            
            //TODO: (Req 10) Pick the correct pipeline state to draw the sky
            // Hints: the sky will be draw after the opaque objects so we would need depth testing but which depth funtion should we pick?
            // We will draw the sphere from the inside, so what options should we pick for the face culling.
            PipelineState skyPipelineState{};
            skyPipelineState.faceCulling.enabled = true;
            skyPipelineState.depthTesting.enabled = true;   
            skyPipelineState.depthTesting.function = GL_LEQUAL;
            // We will draw the sphere of the sky from the inside, so we cull the front faces
            skyPipelineState.faceCulling.culledFace = GL_FRONT;
            skyPipelineState.faceCulling.frontFace = GL_CCW;
        
            
            // Load the sky texture (note that we don't need mipmaps since we want to avoid any unnecessary blurring while rendering the sky)
            std::string skyTextureFile = config.value<std::string>("sky", "");
            Texture2D* skyTexture = texture_utils::loadImage(skyTextureFile, false);

            // Setup a sampler for the sky 
            Sampler* skySampler = new Sampler();
            skySampler->set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            skySampler->set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            skySampler->set(GL_TEXTURE_WRAP_S, GL_REPEAT);
            skySampler->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // Combine all the aforementioned objects (except the mesh) into a material 
            this->skyMaterial = new TexturedMaterial();
            this->skyMaterial->shader = skyShader;
            this->skyMaterial->texture = skyTexture;
            this->skyMaterial->sampler = skySampler;
            this->skyMaterial->pipelineState = skyPipelineState;
            this->skyMaterial->tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            this->skyMaterial->alphaThreshold = 1.0f;
            this->skyMaterial->transparent = false;
        }

        // Then we check if there is a postprocessing shader in the configuration
        if(config.contains("postprocess")){
            
            //TODO: (Req 11) Create a framebuffer
            glGenFramebuffers(1, &postprocessFrameBuffer);
            // glBindFrameBuffer first parameter is the bind target, which could be:
            //  - GL_DRAW_FRAMEBUFFER to which we can draw.
            //  - GL_READ_FRAMEBUFFER from which we can read pixels.
            //  - GL_FRAMEBUFFER which can be used for both reading and drawing.
            // since it's the bind target is not specified, we used the later.
            glBindFramebuffer(GL_FRAMEBUFFER, postprocessFrameBuffer);

            //TODO: (Req 11) Create a color and a depth texture and attach them to the framebuffer
            // Hints: The color format can be (Red, Green, Blue and Alpha components with 8 bits for each channel).
            // The depth format can be (Depth component with 24 bits).
            colorTarget = new Texture2D();
            colorTarget->bind();

            // Allocating the texture memory for the color attachment
            GLuint mip_levels = 1 + std::floor(std::log2(std::max<float>(windowSize.x, windowSize.y)));
            glTexStorage2D(GL_TEXTURE_2D, mip_levels, GL_RGBA8, windowSize.x, windowSize.y);

            // Attaching the color texture to the framebuffer
            // glFramebufferTexture2D takes the following parameters:
            //  - The target framebuffer: the framebuffer type we're targeting (draw, read or both).
            //  - The attachment type: the type of attachment we're going to attach. Right now we're attaching a color attachment. The 0 at the end suggests we can attach more than 1 color attachment.
            //  - The texture target: the type of the texture you want to attach.
            //  - The texture: the actual texture identifier to attach.
            //  - The mipmap level: the mipmap level. We keep this at 0.
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTarget->getOpenGLName(), 0);

            // Doing the same for the depth attachment
            depthTarget = new Texture2D();
            depthTarget->bind();
            // Depth component is a single channel texture, so we only need 1 mipmap level
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, windowSize.x, windowSize.y);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTarget->getOpenGLName(), 0);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                std::cerr << "Error::ForwardRenderer::initialize: Framebuffer is not complete!" << std::endl;
            }
            
            //TODO: (Req 11) Unbind the framebuffer just to be safe
            // We unbind the framebuffer by binding the default framebuffer (0)
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // Create a vertex array to use for drawing the texture
            glGenVertexArrays(1, &postProcessVertexArray);

            // Create a sampler to use for sampling the scene texture in the post processing shader
            Sampler* postprocessSampler = new Sampler();
            postprocessSampler->set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            postprocessSampler->set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            postprocessSampler->set(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            postprocessSampler->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // Create the post processing shader
            ShaderProgram* postprocessShader = new ShaderProgram();
            postprocessShader->attach("assets/shaders/fullscreen.vert", GL_VERTEX_SHADER);
            postprocessShader->attach(config.value<std::string>("postprocess", ""), GL_FRAGMENT_SHADER);
            postprocessShader->link();

            // Create a post processing material
            postprocessMaterial = new TexturedMaterial();
            postprocessMaterial->shader = postprocessShader;
            postprocessMaterial->texture = colorTarget;
            postprocessMaterial->sampler = postprocessSampler;
            // The default options are fine but we don't need to interact with the depth buffer
            // so it is more performant to disable the depth mask
            postprocessMaterial->pipelineState.depthMask = false;
        }

        
    }

    void ForwardRenderer::destroy(){
        // Delete all objects related to the sky
        if(skyMaterial){
            delete skySphere;
            delete skyMaterial->shader;
            delete skyMaterial->texture;
            delete skyMaterial->sampler;
            delete skyMaterial;
        }
        // Delete all objects related to post processing
        if(postprocessMaterial){
            glDeleteFramebuffers(1, &postprocessFrameBuffer);
            glDeleteVertexArrays(1, &postProcessVertexArray);
            delete colorTarget;
            delete depthTarget;
            delete postprocessMaterial->sampler;
            delete postprocessMaterial->shader;
            delete postprocessMaterial;
        }
    }

    void ForwardRenderer::render(World* world){
        // First of all, we search for a camera and for all the mesh renderers
        CameraComponent* camera = nullptr;

        lights.clear();
        opaqueCommands.clear();
        transparentCommands.clear();
        for(auto entity : world->getEntities()){
            // If we hadn't found a camera yet, we look for a camera in this entity
            if(!camera) camera = entity->getComponent<CameraComponent>();
            // If this entity has a mesh renderer component
            if(auto meshRenderer = entity->getComponent<MeshRendererComponent>(); meshRenderer){
                // We construct a command from it
                RenderCommand command;
                command.localToWorld = meshRenderer->getOwner()->getLocalToWorldMatrix();
                command.center = glm::vec3(command.localToWorld * glm::vec4(0, 0, 0, 1));
                command.mesh = meshRenderer->mesh;
                command.material = meshRenderer->material;
                // if it is transparent, we add it to the transparent commands list
                if(command.material->transparent){
                    transparentCommands.push_back(command);
                } else {
                // Otherwise, we add it to the opaque command list
                    opaqueCommands.push_back(command);
                }
            }
            // If this entity has a light component
            if(auto light = entity->getComponent<LightComponent>(); light){
                // We add it to the list of lights
                lights.push_back(light);
            }
        }

        // If there is no camera, we return (we cannot render without a camera)
        if(camera == nullptr) return;

        //TODO: (Req 9) Modify the following line such that "cameraForward" contains a vector pointing the camera forward direction
        // HINT: See how you wrote the CameraComponent::getViewMatrix, it should help you solve this one
        //glm::vec3 cameraForward = glm::vec3(0.0, 0.0, -1.0f);
        // vec4 view camera to -z mult with getlocal to get world view
        glm::vec3 cameraForward =camera->getOwner()->getLocalToWorldMatrix() * glm::vec4(0.0, 0.0, -1.0f,0.0f);
        auto owner = camera->getOwner();
        auto M = owner->getLocalToWorldMatrix();
        glm::vec3 eye = M * glm::vec4(0, 0, 0, 1);
        std::sort(transparentCommands.begin(), transparentCommands.end(), [cameraForward](const RenderCommand& first, const RenderCommand& second){
            //TODO: (Req 9) Finish this function
            // HINT: the following return should return true "first" should be drawn before "second". 

            auto F= glm::dot(cameraForward,first.center); // get distance from camera to first
            auto S= glm::dot(cameraForward,second.center); // get distance from camera to second
            if (F>S) // the fearthest must be drawn first
            {
                return true;
            }
            else{
                return false;
            }
        });

        //TODO: (Req 9) Get the camera ViewProjection matrix and store it in VP 
        // it got camera view and mult with projection to get view projection
        glm::mat4 VP=camera->getProjectionMatrix(windowSize)* camera->getViewMatrix();
        //TODO: (Req 9) Set the OpenGL viewport using viewportStart and viewportSize
        glViewport(0,0,windowSize.x,windowSize.y);
        //TODO: (Req 9) Set the clear color to black and the clear depth to 1
        glClearColor(0.0f,0.0f,0.0f,1.0f);
        glClearDepth(1);
        //TODO: (Req 9) Set the color mask to true and the depth mask to true (to ensure the glClear will affect the framebuffer)
        // it allows to clear color and depth to affect framebuffer
        glColorMask(true,true,true,true);
        glDepthMask(true);

        // If there is a postprocess material, bind the framebuffer
        if(postprocessMaterial){
            //TODO: (Req 11) bind the framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, postprocessFrameBuffer);
        }

        //TODO: (Req 9) Clear the color and depth buffers
        glClearColor(0.0f,0.0f,0.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //TODO: (Req 9) Draw all the opaque commands
        // Don't forget to set the "transform" uniform to be equal the model-view-projection matrix for each render command
        for (auto x: opaqueCommands)
        {
            //Setting materials albedo
            x.material->setup();
            
            // if object's material is lit
            if (auto light_material = dynamic_cast<LitMaterial *>(x.material); light_material)
            {
                //set all uniforms of vertex shader
                light_material->shader->set("VP", VP);
                light_material->shader->set("M", x.localToWorld);
                light_material->shader->set("eye", eye);
                light_material->shader->set("M_IT", glm::transpose(glm::inverse(x.localToWorld)));
                light_material->shader->set("light_count", (int)lights.size());
                
                //loop through the lights vector
                for (int i = 0; i < (int)lights.size(); i++)
                {
                    light_material->shader->set("lights[" + to_string(i) + "].type", static_cast<int>(lights[i]->lightType));

                    light_material->shader->set("lights[" + to_string(i) + "].color",lights[i]->color);

                    //calculate position and direction of the light source from its owner
                    glm::vec3 position = lights[i]->getOwner()->getLocalToWorldMatrix()*glm::vec4(0,0,0,1);
                    glm::vec3 direction = lights[i]->getOwner()->getLocalToWorldMatrix()*glm::vec4(0,-1,0,0);

                    switch (lights[i]->lightType) {
                        case LightType::DIRECTIONAL:
                            light_material->shader->set("lights[" + to_string(i) + "].direction",direction);
                            break;
                        case LightType::POINT:
                            light_material->shader->set("lights[" + to_string(i) + "].position", position); 
                            light_material->shader->set("lights[" + to_string(i) + "].attenuation_constant", lights[i]->attenuation[0]);
                            light_material->shader->set("lights[" + to_string(i) + "].attenuation_linear", lights[i]->attenuation[1]);
                            light_material->shader->set("lights[" + to_string(i) + "].attenuation_quadratic", lights[i]->attenuation[2]);
                            break;
                        case LightType::SPOT:
                            light_material->shader->set("lights[" + to_string(i) + "].position", position); 
                            light_material->shader->set("lights[" + to_string(i) + "].direction",direction);
                            light_material->shader->set("lights[" + to_string(i) + "].attenuation_constant", lights[i]->attenuation[0]);
                            light_material->shader->set("lights[" + to_string(i) + "].attenuation_linear", lights[i]->attenuation[1]);
                            light_material->shader->set("lights[" + to_string(i) + "].attenuation_quadratic", lights[i]->attenuation[2]);
                            light_material->shader->set("lights[" + to_string(i) + "].inner_angle", lights[i]->cone_angles[0]);
                            light_material->shader->set("lights[" + to_string(i) + "].outer_angle", lights[i]->cone_angles[0]);
                            break;
                    }
                    
                    
                    // light_material->shader->set("lights[" + to_string(i) + "].diffuse", lights[i]->diffuse);
                    // light_material->shader->set("lights[" + to_string(i) + "].specular", lights[i]->specular);
                    
                }
            }
            else
            {
                // set transform to be equal the model view projection matrix for each render command
                x.material->shader->set("transform",VP*x.localToWorld);
            }
                        
            x.mesh->draw();
        }
        // If there is a sky material, draw the sky
        if(this->skyMaterial){
            //TODO: (Req 10) setup the sky material
            skyMaterial->setup();
            
            //TODO: (Req 10) Get the camera position
            // it got camera position by mult with local to world to get world position by 0,0,0,1 to get vec4
            glm::vec3 cameraPosition = camera->getOwner()->getLocalToWorldMatrix() * glm::vec4(0.0f, 0.0f, 0.0f,1.0f);
             //glm::vec4 cameraPosition = camera->getViewMatrix() * glm::vec4(0.0f, 0.0f, 0.0f,1.0f);
            //TODO: (Req 10) Create a model matrix for the sy such that it always follows the camera (sky sphere center = camera position)
            // Changes the sky sphere center to camera position
            glm::mat4 modelMatrix = glm::mat4(
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                cameraPosition.x, cameraPosition.y, cameraPosition.z, 1.0f
            );

            
            //TODO: (Req 10) We want the sky to be drawn behind everything (in NDC space, z=1)
            // We can acheive the is by multiplying by an extra matrix after the projection but what values should we put in it?
            glm::mat4 alwaysBehindTransform = glm::mat4(
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 1.0f
            );
            //TODO: (Req 10) set the "transform" uniform
            // The transform uniform is the model-view-projection matrix for the sky sphere
            // We want the sky sphere to be drawn behind everything, so we multiply the model matrix by the alwaysBehindTransform
            // We also need to multiply by the camera view and projection matrices
            skyMaterial->shader->set("transform", alwaysBehindTransform* VP * modelMatrix);
            
            //TODO: (Req 10) draw the sky sphere
            skySphere->draw();
        }
        //TODO: (Req 9) Draw all the transparent commands
        // Don't forget to set the "transform" uniform to be equal the model-view-projection matrix for each render command
        for (auto x: transparentCommands)
        {
            // Setting materials albedo
            x.material->setup();
            // Setting transform uniform to be equal the model view projection matrix for each render command
            x.material->shader->set("transform",VP*x.localToWorld);
            x.mesh->draw();
        }

        // If there is a postprocess material, apply postprocessing
        if(postprocessMaterial){
            //TODO: (Req 11) Return to the default framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            
            //TODO: (Req 11) Setup the postprocess material and draw the fullscreen triangle
            postprocessMaterial->setup();

            // We draw a fullscreen triangle
            glBindVertexArray(postProcessVertexArray);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);

        }

        //if there is a light material
        //call its setup function
        if (MaterialLight)
        {
            MaterialLight->setup();
            
        }
    }

}