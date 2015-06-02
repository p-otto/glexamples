#pragma once

#include <globjects/base/ref_ptr.h>

#include <gloperate/primitives/UniformGroup.h>

template <typename T>
void setUniforms(gloperate::UniformGroup &uniformGroup, const std::string &name, const T &val)
{
    auto uniform = uniformGroup.uniform<T>(name);

    if (!uniform)
    {
        uniformGroup.addUniform(globjects::make_ref<globjects::Uniform<T>>(name));
        uniform = uniformGroup.uniform<T>(name);
    }

    uniform->set(val);
}

template <typename T, typename... Ts>
void setUniforms(gloperate::UniformGroup &uniformGroup, const std::string &name, const T &val, Ts... rest)
{
    setUniforms(uniformGroup, name, val);
    setUniforms(uniformGroup, rest...);
}
