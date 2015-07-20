#include "AmbientOcclusionOptions.h"

#include "AmbientOcclusion.h"
#include <Kernel.h>

AmbientOcclusionOptions::AmbientOcclusionOptions(AmbientOcclusion &painter)
: m_painter(painter)
{
    auto aoOption = m_painter.addProperty<AmbientOcclusionType>("ambient_occlusion", this,
        &AmbientOcclusionOptions::ambientOcclusion,
        &AmbientOcclusionOptions::setAmbientOcclusion);

    aoOption->setChoices({
        None, ScreenSpaceSphere, ScreenSpaceHemisphere, ScreenSpaceDirectional, HorizonBased
    });
    aoOption->setStrings({
        { None, "None" },
        { ScreenSpaceSphere, "SSAO_Sphere" },
        { ScreenSpaceHemisphere, "SSAO_Hemisphere" },
        { ScreenSpaceDirectional, "SSDO" },
        { HorizonBased, "HBAO" }
    });

    auto phong_group = m_painter.addGroup("phong");
    auto ao_group = m_painter.addGroup("ssao");
    auto hbao_group = m_painter.addGroup("hbao");
    auto ssdo_group = m_painter.addGroup("ssdo");
    auto blur_group = m_painter.addGroup("blurring");

    hbao_group->addProperty<int>("number_of_samples", this,
        &AmbientOcclusionOptions::numSamples,
        &AmbientOcclusionOptions::setNumSamples)->setOptions({
            { "minimum", 0 },
            { "maximum", 10 },
            { "step", 1 }
    });

    hbao_group->addProperty<int>("number_of_directions", this,
        &AmbientOcclusionOptions::numDirections,
        &AmbientOcclusionOptions::setNumDirections)->setOptions({
            { "minimum", 0 },
            { "maximum", 10 },
            { "step", 1 }
    });

    ssdo_group->addProperty<float>("color_bleeding_strength", this,
        &AmbientOcclusionOptions::colorBleedingStrength,
        &AmbientOcclusionOptions::setColorBleedingStrength)->setOptions({
            { "minimum", 0.0f },
            { "maximum", 1.0f },
            { "step", 0.1f }
    });

    phong_group->addProperty<float>("ambient", this,
        &AmbientOcclusionOptions::ambient,
        &AmbientOcclusionOptions::setAmbient)->setOptions({
        { "minimum", 0.0f },
        { "maximum", 1.0f },
        { "step", 0.1f }
    });

    phong_group->addProperty<bool>("color", this,
        &AmbientOcclusionOptions::color,
        &AmbientOcclusionOptions::setColor);

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
        { "maximum", 100.0f },
        { "step", 0.5f }
    });
    
    ao_group->addProperty<bool>("half_resolution", this,
        &AmbientOcclusionOptions::halfResolution,
        &AmbientOcclusionOptions::setHalfResolution);

	auto lengthDistributionOption = ao_group->addProperty<Kernel::LengthDistribution>("length_distribution", this,
		&AmbientOcclusionOptions::lengthDistribution,
		&AmbientOcclusionOptions::setLengthDistribution);

	lengthDistributionOption->setChoices({ Kernel::LengthDistribution::Linear, Kernel::LengthDistribution::Quadratic, Kernel::LengthDistribution::Starcraft });
	lengthDistributionOption->setStrings({
		{ Kernel::LengthDistribution::Linear, "Linear" },
		{ Kernel::LengthDistribution::Quadratic, "Quadratic" },
		{ Kernel::LengthDistribution::Starcraft, "Starcraft" }
	});

	auto surfaceDistributionOption = ao_group->addProperty<Kernel::SurfaceDistribution>("surface_distribution", this,
		&AmbientOcclusionOptions::surfaceDistribution,
		&AmbientOcclusionOptions::setSurfaceDistribution);

	surfaceDistributionOption->setChoices({ Kernel::SurfaceDistribution::Random, Kernel::SurfaceDistribution::Uniform });
	surfaceDistributionOption->setStrings({
		{ Kernel::SurfaceDistribution::Random, "Random" },
		{ Kernel::SurfaceDistribution::Uniform, "Uniform" }
	});

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
    m_ambientOcclusionChanged = true;
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

bool AmbientOcclusionOptions::halfResolution() const {
    return m_halfResolution;
}

void AmbientOcclusionOptions::setHalfResolution(bool halfResolution) {
    m_halfResolution = halfResolution;
    m_resolutionChanged = true;
}

Kernel::KernelType AmbientOcclusionOptions::kernelType() const {
	return m_kernelType;
}

void AmbientOcclusionOptions::setKernelType(Kernel::KernelType type) {
	m_kernelType = type;
	m_kernelChanged = true;
}

Kernel::LengthDistribution AmbientOcclusionOptions::lengthDistribution() const {
	return m_lengthDistribution;
}

void AmbientOcclusionOptions::setLengthDistribution(Kernel::LengthDistribution length) {
	m_lengthDistribution = length;
	m_kernelChanged = true;
}

Kernel::SurfaceDistribution AmbientOcclusionOptions::surfaceDistribution() const {
	return m_surfaceDistribution;
}

void AmbientOcclusionOptions::setSurfaceDistribution(Kernel::SurfaceDistribution surface) {
	m_surfaceDistribution = surface;
	m_kernelChanged = true;
}

bool AmbientOcclusionOptions::biliteralBlurring() const
{
    return m_biliteralBlurring;
}

void AmbientOcclusionOptions::setBiliteralBlurring(bool biliteralBlurring)
{
    m_biliteralBlurring = biliteralBlurring;
}

float AmbientOcclusionOptions::ambient() const
{
    return m_ambientTerm;
}

void AmbientOcclusionOptions::setAmbient(float ambient)
{
    m_ambientTerm = ambient;
}

bool AmbientOcclusionOptions::color() const
{
    return m_color;
}

void AmbientOcclusionOptions::setColor(bool color)
{
    m_color = color;
}

int AmbientOcclusionOptions::numSamples() const
{
    return m_numSamples;
}

void AmbientOcclusionOptions::setNumSamples(int numSamples)
{
    m_numSamples = numSamples;
}

int AmbientOcclusionOptions::numDirections() const
{
    return m_numDirections;
}

void AmbientOcclusionOptions::setNumDirections(int numDirections)
{
    m_numDirections = numDirections;
}

float AmbientOcclusionOptions::colorBleedingStrength() const
{
    return m_colorBleedingStrength;
}

void AmbientOcclusionOptions::setColorBleedingStrength(float colorBleedingStrength)
{
    m_colorBleedingStrength = colorBleedingStrength;
}

int AmbientOcclusionOptions::rotationTexSize() const
{
    return m_rotationTexSize;
}

float AmbientOcclusionOptions::minimalKernelLength() const
{
    return m_minimalKernelLength;
}

float AmbientOcclusionOptions::minimalKernelAngle() const
{
    return m_minimalKernelAngle;
}

bool AmbientOcclusionOptions::hasResolutionChanged()
{
    if (m_resolutionChanged) {
        m_resolutionChanged = false;
        return true;
    }
    return false;
}

bool AmbientOcclusionOptions::hasAmbientOcclusionTypeChanged()
{
    if (m_ambientOcclusionChanged)
    {
        m_ambientOcclusionChanged = false;
        return true;
    }
    return false;
}

bool AmbientOcclusionOptions::hasKernelChanged() {
	if (m_kernelChanged) {
		m_kernelChanged = false;
		return true;
	}
	return false;
}
