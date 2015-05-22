#include "AmbientOcclusionOptions.h"

#include "AmbientOcclusion.h"

AmbientOcclusionOptions::AmbientOcclusionOptions(AmbientOcclusion &painter)
: m_painter(painter)
{
    m_painter.addProperty<int>("kernel_size", this,
        &AmbientOcclusionOptions::kernelSize,
        &AmbientOcclusionOptions::setKernelSize)->setOptions({
        { "minimum", 0 },
        { "maximum", maxKernelSize() },
        { "step", 1 }
    });
    
    m_painter.addProperty<float>("kernel_radius", this,
        &AmbientOcclusionOptions::kernelRadius,
        &AmbientOcclusionOptions::setKernelRadius)->setOptions({
        { "minimum", 0.2f },
        { "maximum", 8.0f },
        { "step", 0.5f }
    });
    
    m_painter.addProperty<int>("blur_kernel_size", this,
        &AmbientOcclusionOptions::blurKernelSize,
        &AmbientOcclusionOptions::setblurKernelSize)->setOptions({
        { "minimum", 0 },
        { "maximum", 7 },
        { "step", 1 }
    });

    m_painter.addProperty<bool>("half_resolution", this,
        &AmbientOcclusionOptions::halfResolution,
        &AmbientOcclusionOptions::setHalfResolution);
    
    m_painter.addProperty<bool>("normal_oriented", this,
        &AmbientOcclusionOptions::normalOriented,
        &AmbientOcclusionOptions::setNormalOriented);
    
    m_painter.addProperty<bool>("attenuation", this,
        &AmbientOcclusionOptions::attenuation,
        &AmbientOcclusionOptions::setAttenuation);
}

int AmbientOcclusionOptions::maxKernelSize() const
{
    return s_maxKernelSize;
}

int AmbientOcclusionOptions::kernelSize() const
{
    return m_kernelSize;
}

void AmbientOcclusionOptions::setKernelSize(int kernelSize)
{
    m_kernelSize = kernelSize;
}

int AmbientOcclusionOptions::blurKernelSize() const
{
    return m_blurKernelSize;
}

void AmbientOcclusionOptions::setblurKernelSize(int blurKernelSize)
{
    m_blurKernelSize = blurKernelSize;
}

float AmbientOcclusionOptions::kernelRadius() const
{
    return m_kernelRadius;
}

void AmbientOcclusionOptions::setKernelRadius(float kernelRadius)
{
    m_kernelRadius = kernelRadius;
}

bool AmbientOcclusionOptions::normalOriented() const
{
    return m_normalOriented;
}

void AmbientOcclusionOptions::setNormalOriented(bool normalOriented)
{
    m_normalOriented = normalOriented;
    m_painter.setupKernelAndRotationTex();
}

bool AmbientOcclusionOptions::attenuation() const
{
    return m_attenuation;
}

void AmbientOcclusionOptions::setAttenuation(bool attenuation)
{
    m_attenuation = attenuation;
}

bool AmbientOcclusionOptions::halfResolution() const {
    return m_halfResolution;
}

void AmbientOcclusionOptions::setHalfResolution(bool halfResolution) {
    m_halfResolution = halfResolution;
    m_resolutionChanged = true;
}

int AmbientOcclusionOptions::rotationTexSize() const
{
    return m_rotationTexSize;
}

float AmbientOcclusionOptions::minimalKernelLength() const
{
    return m_minimalKernelLength;
}

float AmbientOcclusionOptions::minimalKernelAngle() const{
    return m_minimalKernelAngle;
}

bool AmbientOcclusionOptions::hasResolutionChanged() {
    if (m_resolutionChanged) {
        m_resolutionChanged = false;
        return true;
    }
    return false;
}
