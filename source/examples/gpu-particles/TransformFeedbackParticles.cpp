
#include "TransformFeedbackParticles.h"

#include <glbinding/gl/gl.h>

#include <globjects/Shader.h>
#include <globjects/VertexAttributeBinding.h>

#include <globjects/base/File.h>

#include <common/Camera.h>


using namespace gl;
using namespace glm;


TransformFeedbackParticles::TransformFeedbackParticles(
    const std::vector<vec4> & positions
,   const std::vector<vec4> & velocities
,   const globjects::Texture & forces
,   const Camera & camera)
: AbstractParticleTechnique(positions, velocities, forces, camera)
{
}

TransformFeedbackParticles::~TransformFeedbackParticles()
{
}

void TransformFeedbackParticles::initialize()
{
    m_sourcePositions  = new globjects::Buffer();
    m_sourceVelocities = new globjects::Buffer();
    m_targetPositions  = new globjects::Buffer();
    m_targetVelocities = new globjects::Buffer();

    reset();

    m_transformFeedbackProgram = new globjects::Program();
    m_transformFeedbackProgram->attach(globjects::Shader::fromFile(GL_VERTEX_SHADER, "data/gpu-particles/transformfeedback.vert"));

    m_transformFeedbackProgram->link();

    m_transformFeedback = new globjects::TransformFeedback();
    m_transformFeedback->setVaryings(m_transformFeedbackProgram, std::array<const char *, 2>{ { "out_position", "out_velocity" } }, GL_SEPARATE_ATTRIBS);

    m_vao = new globjects::VertexArray();
    m_vao->bind();

    auto positionsBinding = m_vao->binding(0);
    positionsBinding->setAttribute(0);
    positionsBinding->setFormat(4, GL_FLOAT);
    m_vao->enable(0);

    auto velocitiesBinding = m_vao->binding(1);
    velocitiesBinding->setAttribute(1);
    velocitiesBinding->setFormat(4, GL_FLOAT);
    m_vao->enable(1);

    m_vao->unbind();


    AbstractParticleTechnique::initialize("data/gpu-particles/points.vert");
}

void TransformFeedbackParticles::reset()
{
    m_sourcePositions->setData(m_positions, GL_DYNAMIC_DRAW);
    m_targetPositions->setData(static_cast<GLsizei>(m_numParticles * sizeof(vec4)), nullptr, GL_DYNAMIC_DRAW);

    m_sourceVelocities->setData(m_velocities, GL_DYNAMIC_DRAW);
    m_targetVelocities->setData(static_cast<GLsizei>(m_numParticles * sizeof(vec4)), nullptr, GL_DYNAMIC_DRAW);

    AbstractParticleTechnique::reset();
}

void TransformFeedbackParticles::step(const float elapsed)
{
    m_vao->bind();

    m_vao->binding(0)->setBuffer(m_sourcePositions,  0, sizeof(vec4));
    m_vao->binding(1)->setBuffer(m_sourceVelocities, 0, sizeof(vec4));

    m_targetPositions->bindBase (GL_TRANSFORM_FEEDBACK_BUFFER, 0);
    m_targetVelocities->bindBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1);

    m_forces.bind();
    m_transformFeedbackProgram->setUniform("forces", 0);
    m_transformFeedbackProgram->setUniform("elapsed", elapsed);

    glEnable(GL_RASTERIZER_DISCARD);

    m_transformFeedbackProgram->use();
    m_transformFeedback->bind();
    m_transformFeedback->begin(GL_POINTS);
    m_vao->drawArrays(GL_POINTS, 0, m_numParticles);
    m_transformFeedback->end();
    m_transformFeedback->unbind();
    m_transformFeedbackProgram->release();

    glDisable(GL_RASTERIZER_DISCARD);

    m_vao->unbind();

    m_forces.unbind();

    std::swap(m_sourcePositions, m_targetPositions);
    std::swap(m_sourceVelocities, m_targetVelocities);
}

void TransformFeedbackParticles::draw_impl()
{
    m_vao->binding(0)->setBuffer(m_sourcePositions, 0, sizeof(vec4));
    m_vao->binding(1)->setBuffer(m_sourceVelocities, 0, sizeof(vec4));

    m_drawProgram->use();

    m_vao->bind();
    //m_vao->drawArrays(GL_POINTS, 0, m_numParticles); // don't use this, it is erroneous
    m_transformFeedback->draw(GL_POINTS);
    m_vao->unbind();

    m_drawProgram->release();
}
