#include "AmbientOcclusion.h"

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/bitfield.h>
#include <glbinding/gl/boolean.h>

#include <globjects/globjects.h>
#include <globjects/logging.h>
#include <globjects/DebugMessage.h>
#include <globjects/Program.h>
#include <globjects/Texture.h>

#include <gloperate/base/RenderTargetType.h>

#include <gloperate/painter/TargetFramebufferCapability.h>
#include <gloperate/painter/ViewportCapability.h>
#include <gloperate/painter/PerspectiveProjectionCapability.h>
#include <gloperate/painter/CameraCapability.h>
#include <gloperate/painter/VirtualTimeCapability.h>

#include <gloperate/primitives/AdaptiveGrid.h>
#include <gloperate/base/make_unique.hpp>

using namespace gl;
using namespace glm;
using namespace globjects;

AmbientOcclusion::AmbientOcclusion(gloperate::ResourceManager & resourceManager)
:   Painter(resourceManager)
,   m_targetFramebufferCapability(addCapability(new gloperate::TargetFramebufferCapability()))
,   m_viewportCapability(addCapability(new gloperate::ViewportCapability()))
,   m_projectionCapability(addCapability(new gloperate::PerspectiveProjectionCapability(m_viewportCapability)))
,   m_cameraCapability(addCapability(new gloperate::CameraCapability()))
{
}

AmbientOcclusion::~AmbientOcclusion() = default;

void AmbientOcclusion::setupProjection()
{
    static const auto zNear = 0.3f, zFar = 50.f, fovy = 50.f;

    m_projectionCapability->setZNear(zNear);
    m_projectionCapability->setZFar(zFar);
    m_projectionCapability->setFovy(radians(fovy));

    m_grid->setNearFar(zNear, zFar);
}

void AmbientOcclusion::setupFramebuffers()
{
    if (m_multisampling)
    {
        m_colorAttachment = new Texture(GL_TEXTURE_2D_MULTISAMPLE);
        m_colorAttachment->bind(); // workaround
        m_normalDepthAttachment = new Texture(GL_TEXTURE_2D_MULTISAMPLE);
        m_normalDepthAttachment->bind();
        m_depthBuffer = new Texture(GL_TEXTURE_2D_MULTISAMPLE);
        m_depthBuffer->bind();
        
        m_occlusionAttachment = new Texture(GL_TEXTURE_2D_MULTISAMPLE);
        m_occlusionAttachment->bind();
        
        m_blurAttachment = new Texture(GL_TEXTURE_2D_MULTISAMPLE);
        m_blurAttachment->bind();
    }
    else
    {
        m_colorAttachment = Texture::createDefault(GL_TEXTURE_2D);
        m_normalDepthAttachment = Texture::createDefault(GL_TEXTURE_2D);
        m_depthBuffer = Texture::createDefault(GL_TEXTURE_2D);
        
        m_occlusionAttachment = Texture::createDefault(GL_TEXTURE_2D);
        
        m_blurAttachment = Texture::createDefault(GL_TEXTURE_2D);
    }
    
    m_modelFbo = make_ref<Framebuffer>();
    m_modelFbo->attachTexture(GL_COLOR_ATTACHMENT0, m_colorAttachment);
    m_modelFbo->attachTexture(GL_COLOR_ATTACHMENT1, m_normalDepthAttachment);
    m_modelFbo->attachTexture(GL_DEPTH_ATTACHMENT, m_depthBuffer);
    m_modelFbo->setDrawBuffers({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1});
    
    m_occlusionFbo = make_ref<Framebuffer>();
    m_occlusionFbo->attachTexture(GL_COLOR_ATTACHMENT0, m_occlusionAttachment);
    
    m_blurFbo = make_ref<Framebuffer>();
    m_blurFbo->attachTexture(GL_COLOR_ATTACHMENT0, m_blurAttachment);
    
    updateFramebuffers();
    
    m_modelFbo->printStatus(true);
    m_occlusionFbo->printStatus(true);
}

void AmbientOcclusion::setupModel()
{
    const auto meshLoader = gloperate_assimp::AssimpMeshLoader{};
    const auto scene = meshLoader.load("data/ambientocclusion/teapot.obj", nullptr);
    m_model = gloperate::make_unique<gloperate::PolygonalDrawable>(*scene);
    
    m_grid = make_ref<gloperate::AdaptiveGrid>();
    m_grid->setColor({0.6f, 0.6f, 0.6f});
}

void AmbientOcclusion::setupShaders()
{
    m_modelProgram = new Program{};
    m_modelProgram->attach(
                           Shader::fromFile(GL_VERTEX_SHADER, "data/ambientocclusion/model.vert"),
                           Shader::fromFile(GL_FRAGMENT_SHADER, "data/ambientocclusion/model.frag")
    );
    
    m_ambientOcclusionProgram = new Program{};
    m_ambientOcclusionProgram->attach(
                            Shader::fromFile(GL_VERTEX_SHADER, "data/ambientocclusion/screen_quad.vert"),
                            Shader::fromFile(GL_FRAGMENT_SHADER, "data/ambientocclusion/ssao_crytek.frag")
    );
    
    m_blurProgram = new Program{};
    m_blurProgram->attach(
                            Shader::fromFile(GL_VERTEX_SHADER, "data/ambientocclusion/screen_quad.vert"),
                            Shader::fromFile(GL_FRAGMENT_SHADER, "data/ambientocclusion/blur.frag")
    );
    
    m_mixProgram = new Program{};
    m_mixProgram->attach(
                            Shader::fromFile(GL_VERTEX_SHADER, "data/ambientocclusion/screen_quad.vert"),
                            Shader::fromFile(GL_FRAGMENT_SHADER, "data/ambientocclusion/mix.frag")
    );
}

void AmbientOcclusion::updateFramebuffers()
{
    static const auto numSamples = 4u;
    const auto width = m_viewportCapability->width(), height = m_viewportCapability->height();
    
    if (m_multisampling)
    {
        m_colorAttachment->image2DMultisample(numSamples, GL_RGBA8, width, height, GL_TRUE);
        m_normalDepthAttachment->image2DMultisample(numSamples, GL_RGBA8, width, height, GL_TRUE);
        m_depthBuffer->image2DMultisample(numSamples, GL_DEPTH_COMPONENT, width, height, GL_TRUE);
        
        m_occlusionAttachment->image2DMultisample(numSamples, GL_R8, width, height, GL_TRUE);
        
        m_blurAttachment->image2DMultisample(numSamples, GL_R8, width, height, GL_TRUE);
    }
    else
    {
        m_colorAttachment->image2D(0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        m_normalDepthAttachment->image2D(0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        m_depthBuffer->image2D(0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
        
        m_occlusionAttachment->image2D(0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
        
        m_blurAttachment->image2D(0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    }
}

std::vector<glm::vec3> AmbientOcclusion::getCrytekKernel(int size)
{
    srand(clock());
    std::vector<glm::vec3> kernel(size);
    int count = 1;
    
    for (auto &vec : kernel)
    {
        // TODO: use C++ <random>
        do {
            vec[0] = static_cast<float>(rand() % 1024) / 512.0f - 1.0f;
            vec[1] = static_cast<float>(rand() % 1024) / 512.0f - 1.0f;
            vec[2] = static_cast<float>(rand() % 1024) / 1024.0f;
        } while(glm::dot(vec, glm::vec3(0,0,1)) < 0.1f);
        
        vec = glm::normalize(vec);
        
        float scale = count++ / size;
        scale = glm::mix(m_minimalKernelLength, 1.0f, scale * scale);
        vec *= scale;
    }
    
    return kernel;
}

std::vector<glm::vec3> AmbientOcclusion::getRotationTexture(int size)
{
    srand(clock());
    std::vector<glm::vec3> tex(size * size);
    
    for (auto &vec : tex)
    {
        // TODO: use C++ <random>
        vec[0] = static_cast<float>(rand() % 1024) / 512.0f - 1.0f;
        vec[1] = static_cast<float>(rand() % 1024) / 512.0f - 1.0f;
        vec[2] = 0.0f;
        
        vec = glm::normalize(vec);
    }
    
    return tex;
}

void AmbientOcclusion::onInitialize()
{
    // create program
    globjects::init();
    
#ifdef __APPLE__
    Shader::clearGlobalReplacements();
    Shader::globalReplace("#version 140", "#version 150");

    debug() << "Using global OS X shader replacement '#version 140' -> '#version 150'" << std::endl;
#endif

    glClearColor(0.85f, 0.87f, 0.91f, 1.0f);

    // some magic numbers that give a good view on the teapot
    m_cameraCapability->setEye(glm::vec3(0.0f, 1.7f, -2.0f));
    m_cameraCapability->setCenter(glm::vec3(0.2f, 0.3f, 0.0f));
    
    m_rotationTex = Texture::createDefault(GL_TEXTURE_2D);
    m_rotationTex->setParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
    m_rotationTex->setParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
    m_rotationTex->setParameter(GL_TEXTURE_WRAP_R, GL_REPEAT);
    
    std::vector<glm::vec3> rotationValues = getRotationTexture(m_rotationTexSize);
    m_rotationTex->image2D(0, GL_RGB32F, m_rotationTexSize, m_rotationTexSize, 0, GL_RGB, GL_FLOAT, rotationValues.data());
    
    setupFramebuffers();
    setupModel();
    setupShaders();
    setupProjection();
}

void AmbientOcclusion::onPaint()
{
    if (m_viewportCapability->hasChanged())
    {
        glViewport(
            m_viewportCapability->x(),
            m_viewportCapability->y(),
            m_viewportCapability->width(),
            m_viewportCapability->height());

        m_viewportCapability->setChanged(false);
		updateFramebuffers();
    }

    glEnable(GL_DEPTH_TEST);
    
    m_modelFbo->bind(GL_FRAMEBUFFER);
    m_modelFbo->clearBuffer(GL_COLOR, 0, glm::vec4{0.85f, 0.87f, 0.91f, 1.0f});
    m_modelFbo->clearBuffer(GL_COLOR, 1, glm::vec4{0.0f, 0.0f, 0.0f, 1.0f});
    m_modelFbo->clearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);
    
    const auto transform = m_projectionCapability->projection() * m_cameraCapability->view();
    const auto eye = m_cameraCapability->eye();

    m_modelProgram->use();
    
    m_modelProgram->setUniform("u_mvp", transform);
    m_modelProgram->setUniform("u_view", m_cameraCapability->view());
    m_model->draw();
    
    mat4 model;
    model = glm::translate(model, glm::vec3(0.5, 0.0, 1.3));
    m_modelProgram->setUniform("u_view", m_cameraCapability->view());
    m_modelProgram->setUniform("u_mvp", transform * model);
    m_model->draw();
    
    m_grid->update(eye, transform);
    m_grid->draw();
    
    m_modelProgram->release();
    
    glDisable(GL_DEPTH_TEST);
    
    // calculate ambient occlusion
    m_screenAlignedQuad = new gloperate::ScreenAlignedQuad(m_ambientOcclusionProgram);
    m_screenAlignedQuad->program()->use();
    
    m_occlusionFbo->bind();
    m_occlusionFbo->clearBuffer(GL_COLOR, 0, glm::vec4{0.0, 0.0, 0.0, 0.0});
    
    m_normalDepthAttachment->bindActive(GL_TEXTURE0);
    m_screenAlignedQuad->program()->setUniform("u_normal_depth", 0);
    m_rotationTex->bindActive(GL_TEXTURE1);
    m_screenAlignedQuad->program()->setUniform("u_rotation", 1);
    
    glm::mat4 invProj = glm::inverse(m_projectionCapability->projection());
    glm::mat4 invView = glm::inverse(m_cameraCapability->view());
    m_screenAlignedQuad->program()->setUniform("u_invProj", invProj);
    m_screenAlignedQuad->program()->setUniform("u_invView", invView);
    m_screenAlignedQuad->program()->setUniform("u_proj", m_projectionCapability->projection());
    m_screenAlignedQuad->program()->setUniform("u_resolutionX", m_viewportCapability->width());
    m_screenAlignedQuad->program()->setUniform("u_resolutionY", m_viewportCapability->height());
    
    static const auto kernel = getCrytekKernel(m_kernelSize);
    glProgramUniform3fv(m_screenAlignedQuad->program()->id(), m_screenAlignedQuad->program()->getUniformLocation("kernel"), kernel.size(), glm::value_ptr(kernel[0]));
    
    m_screenAlignedQuad->draw();
    
    // blur ambient occlusion texture
	blur(m_occlusionAttachment, m_blurFbo);
    
    // finally, render to screen
    auto default_framebuffer = m_targetFramebufferCapability->framebuffer();
    if (!default_framebuffer) {
        default_framebuffer = globjects::Framebuffer::defaultFBO();
    }
    
    default_framebuffer->bind(GL_FRAMEBUFFER);
    default_framebuffer->clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    m_screenAlignedQuad = new gloperate::ScreenAlignedQuad(m_mixProgram);
    m_screenAlignedQuad->program()->use();
    
    m_colorAttachment->bindActive(GL_TEXTURE0);
    m_screenAlignedQuad->program()->setUniform("u_color", 0);
    m_blurAttachment->bindActive(GL_TEXTURE1);
    m_screenAlignedQuad->program()->setUniform("u_blur", 1);
    
    m_screenAlignedQuad->draw();
}


void AmbientOcclusion::blur(globjects::Texture *input, globjects::Framebuffer *output) {
	// TODO: two-pass blur

	m_screenAlignedQuad = new gloperate::ScreenAlignedQuad(m_blurProgram);
	m_screenAlignedQuad->program()->use();

	output->bind();
	output->clearBuffer(GL_COLOR, 0, glm::vec4{ 0.0, 0.0, 0.0, 0.0 });

	input->bindActive(GL_TEXTURE0);
	m_screenAlignedQuad->program()->setUniform("u_occlusion", 0);
    m_normalDepthAttachment->bindActive(GL_TEXTURE1);
    m_screenAlignedQuad->program()->setUniform("u_normal_depth", 1);
    
	m_screenAlignedQuad->draw();
}
