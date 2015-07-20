#pragma once
#include <Kernel.h>

class AmbientOcclusion;

enum AmbientOcclusionType { None, ScreenSpaceSphere, ScreenSpaceHemisphere, ScreenSpaceDirectional, HorizonBased };

class AmbientOcclusionOptions
{
public:
    AmbientOcclusionOptions(AmbientOcclusion &painter);
    ~AmbientOcclusionOptions() = default;
    
    void addProperties();
    
    AmbientOcclusionType ambientOcclusion() const;
    void setAmbientOcclusion(AmbientOcclusionType ambientOcclusion);

    // kernel settings
    int maxKernelSize() const;
    
    int kernelSize() const;
    void setKernelSize(int kernelSize);
    
    float kernelRadius() const;
    void setKernelRadius(float kernelRadius);

    bool halfResolution() const;
    void setHalfResolution(bool halfResolution);

	Kernel::KernelType kernelType() const;
	void setKernelType(Kernel::KernelType type);

	Kernel::LengthDistribution lengthDistribution() const;
	void setLengthDistribution(Kernel::LengthDistribution type);

	Kernel::SurfaceDistribution surfaceDistribution() const;
	void setSurfaceDistribution(Kernel::SurfaceDistribution type);

    // hbao settings
    int numDirections() const;
    void setNumDirections(int numDirections);

    int numSamples() const;
    void setNumSamples(int numSamples);

    // ssdo settings
    float colorBleedingStrength() const;
    void setColorBleedingStrength(float colorBleedingStrength);

	// blur settings

	int blurKernelSize() const;
	void setblurKernelSize(int blurKernelSize);

    bool biliteralBlurring() const;
    void setBiliteralBlurring(bool biliteralBlurring);

    // phong settings
    float ambient() const;
    void setAmbient(float ambient);

    bool color() const;
    void setColor(bool color);

    // TODO: implement setters and (if necessary update AmbientOcclusion)
    int rotationTexSize() const;
    float minimalKernelLength() const;
    float minimalKernelAngle() const;

    bool hasResolutionChanged();
    bool hasAmbientOcclusionTypeChanged();
	bool hasKernelChanged();
    
private:
    AmbientOcclusion & m_painter;
    
    static const int s_maxKernelSize = 128;
    
    AmbientOcclusionType m_ambientOcclusion = AmbientOcclusionType::ScreenSpaceHemisphere;
    
    int m_kernelSize = 32;
    int m_blurKernelSize = 7;
    float m_kernelRadius = 7.0f;
    bool m_halfResolution = true;
    bool m_biliteralBlurring = true;

    float m_ambientTerm = 1.f;
    bool m_color = false;
    
    int m_rotationTexSize = 4;
    float m_minimalKernelLength = 0.1f;
    float m_minimalKernelAngle = 0.1f;

    int m_numSamples = 5;
    int m_numDirections = 6;

    float m_colorBleedingStrength = 1.0;

    bool m_resolutionChanged = false;
    bool m_ambientOcclusionChanged = false;
	bool m_kernelChanged = false;

	Kernel::KernelType m_kernelType = Kernel::KernelType::Hemisphere;
	Kernel::LengthDistribution m_lengthDistribution = Kernel::LengthDistribution::Quadratic;
	Kernel::SurfaceDistribution m_surfaceDistribution = Kernel::SurfaceDistribution::Random;
	// TODO: Add parameters for Kernel
};
