#pragma once

class AmbientOcclusion;

class AmbientOcclusionOptions
{
public:
    AmbientOcclusionOptions(AmbientOcclusion &painter);
    ~AmbientOcclusionOptions() = default;
    
    void addProperties();
    
    int maxKernelSize() const;
    
    int kernelSize() const;
    void setKernelSize(int kernelSize);
    
    int blurKernelSize() const;
    void setblurKernelSize(int blurKernelSize);
    
    bool normalOriented() const;
    void setNormalOriented(bool normalOriented);
    
    float kernelRadius() const;
    void setKernelRadius(float kernelRadius);
    
    bool attenuation() const;
    void setAtteunuation(bool attenuation);
    
    // TODO: implement setters and (if necessary update AmbientOcclusion)
    int rotationTexSize() const;
    float minimalKernelLength() const;
    float minimalKernelAngle() const;
    
private:
    AmbientOcclusion & m_painter;
    
    static const int s_maxKernelSize = 128;
    
    int m_kernelSize = 32;
    int m_blurKernelSize = 2;
    bool m_normalOriented = true;
    float m_kernelRadius = 1.0f;
    bool m_attenuation = false;
    
    int m_rotationTexSize = 4;
    float m_minimalKernelLength = 0.1f;
    float m_minimalKernelAngle = 0.1f;
};
