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
    
    m_painter.addProperty<bool>("normal_oriented", this,
        &AmbientOcclusionOptions::normalOriented,
        &AmbientOcclusionOptions::setNormalOriented);
    
    m_painter.addProperty<bool>("attenuation", this,
                                &AmbientOcclusionOptions::attenuation,
                                &AmbientOcclusionOptions::setAtteunuation);
    
    m_painter.addProperty<int>("blur_kernel_size", this,
        &AmbientOcclusionOptions::blurKernelSize,
        &AmbientOcclusionOptions::setblurKernelSize)->setOptions({
        { "minimum", 0 },
        { "maximum", 7 },
        { "step", 1 }
    });
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

bool AmbientOcclusionOptions::normalOriented() const
{
    return m_normalOriented;
}

float AmbientOcclusionOptions::kernelRadius() const
{
    return m_kernelRadius;
}

void AmbientOcclusionOptions::setKernelRadius(float kernelRadius)
{
    m_kernelRadius = kernelRadius;
}

void AmbientOcclusionOptions::setNormalOriented(bool normalOriented)
{
    m_normalOriented = normalOriented;
}

bool AmbientOcclusionOptions::attenuation() const
{
    return m_attenuation;
}

void AmbientOcclusionOptions::setAtteunuation(bool attenuation)
{
    m_attenuation = attenuation;
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
