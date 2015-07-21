#include <gloperate/plugin/plugin_api.h>

#include "AmbientOcclusion.h"

#include <glexamples-version.h>

GLOPERATE_PLUGIN_LIBRARY

    GLOPERATE_PAINTER_PLUGIN(AmbientOcclusion
    , "AmbientOcclusion"
    , "Screen-Space Ambient Occlusion Techniques"
    , GLEXAMPLES_AUTHOR_ORGANIZATION
    , "v1.0.0")

GLOPERATE_PLUGIN_LIBRARY_END
