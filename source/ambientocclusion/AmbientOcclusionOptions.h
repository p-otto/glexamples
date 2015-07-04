#pragma once

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
    
    int blurKernelSize() const;
    void setblurKernelSize(int blurKernelSize);
    
    float kernelRadius() const;
    void setKernelRadius(float kernelRadius);

    bool halfResolution() const;
    void setHalfResolution(bool halfResolution);

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
    
private:
    AmbientOcclusion & m_painter;
    
    static const int s_maxKernelSize = 128;
    
    AmbientOcclusionType m_ambientOcclusion = AmbientOcclusionType::ScreenSpaceHemisphere;
    
    int m_kernelSize = 32;
    int m_blurKernelSize = 7;
    float m_kernelRadius = 7.0f;
    bool m_halfResolution = true;
    bool m_biliteralBlurring = true;

    float m_ambientTerm = 0.8f;
    bool m_color = false;
    
    int m_rotationTexSize = 4;
    float m_minimalKernelLength = 0.1f;
    float m_minimalKernelAngle = 0.1f;

    bool m_resolutionChanged = false;
    bool m_ambientOcclusionChanged = false;
};
