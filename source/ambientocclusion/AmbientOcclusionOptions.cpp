#include "AmbientOcclusionOptions.h"

#include "AmbientOcclusion.h"

AmbientOcclusionOptions::AmbientOcclusionOptions(AmbientOcclusion &painter)
: m_painter(painter)
{
    auto aoOption = m_painter.addProperty<AmbientOcclusionType>("ambient_occlusion", this,
        &AmbientOcclusionOptions::ambientOcclusion,
        &AmbientOcclusionOptions::setAmbientOcclusion);

    aoOption->setChoices({
        None, ScreenSpaceSphere, ScreenSpaceHemisphere
    });
    aoOption->setStrings({
        { None, "None" },
        { ScreenSpaceSphere, "SSAO_Sphere" },
        { ScreenSpaceHemisphere, "SSAO_Hemisphere" }
    });

    m_painter.addProperty<bool>("phong", this,
        &AmbientOcclusionOptions::phong,
        &AmbientOcclusionOptions::setPhong);

    auto ao_group = m_painter.addGroup("ssao");
    auto blur_group = m_painter.addGroup("blurring");

    ao_group->addProperty<int>("kernel_size", this,
        &AmbientOcclusionOptions::kernelSize,
        &AmbientOcclusionOptions::setKernelSize)->setOptions({
        { "minimum", 0 },
        { "maximum", maxKernelSize() },
        { "step", 1 }
    });
    
    ao_group->addProperty<float>("kernel_radius", this,
        &AmbientOcclusionOptions::kernelRadius,
        &AmbientOcclusionOptions::setKernelRadius)->setOptions({
        { "minimum", 0.2f },
        { "maximum", 8.0f },
        { "step", 0.5f }
    });
    
    ao_group->addProperty<bool>("half_resolution", this,
        &AmbientOcclusionOptions::halfResolution,
        &AmbientOcclusionOptions::setHalfResolution);

    ao_group->addProperty<bool>("attenuation", this,
        &AmbientOcclusionOptions::attenuation,
        &AmbientOcclusionOptions::setAttenuation);

    blur_group->addProperty<int>("blur_kernel_size", this,
        &AmbientOcclusionOptions::blurKernelSize,
        &AmbientOcclusionOptions::setblurKernelSize)->setOptions({
        { "minimum", 0 },
        { "maximum", 7 },
        { "step", 1 }
    });

    blur_group->addProperty<bool>("biliteral_blurring", this,
        &AmbientOcclusionOptions::biliteralBlurring,
        &AmbientOcclusionOptions::setBiliteralBlurring);
}

AmbientOcclusionType AmbientOcclusionOptions::ambientOcclusion() const {
    return m_ambientOcclusion;
}

void AmbientOcclusionOptions::setAmbientOcclusion(AmbientOcclusionType ambientOcclusion) {
    m_ambientOcclusion = ambientOcclusion;
    m_painter.setAmbientOcclusion(ambientOcclusion);
}

bool AmbientOcclusionOptions::phong() const {
    return m_phong;
}

void AmbientOcclusionOptions::setPhong(bool phong) {
    m_phong = phong;
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

bool AmbientOcclusionOptions::biliteralBlurring() const
{
    return m_biliteralBlurring;
}

void AmbientOcclusionOptions::setBiliteralBlurring(bool biliteralBlurring)
{
    m_biliteralBlurring = biliteralBlurring;
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
