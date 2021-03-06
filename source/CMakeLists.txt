
# Generate meta-header
configure_file(glexamples-version.h.in ${CMAKE_CURRENT_BINARY_DIR}/glexamples-version.h)

# We do NOT want to deploy this header (at least not for binary packages)
#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/template/template-version.h DESTINATION ${INSTALL_INCLUDE}/template)

# External libraries
find_package(OpenGL REQUIRED)
find_package(GLM REQUIRED)
find_package(glbinding REQUIRED)
find_package(globjects REQUIRED)
find_package(gloperate REQUIRED)
find_package(libzeug REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}

    ${GLM_INCLUDE_DIR}
    ${GLBINDING_INCLUDE_DIRS}
    ${GLOBJECTS_INCLUDE_DIRS}
    ${GLOPERATE_INCLUDE_DIRS}
    ${LIBZEUG_INCLUDE_DIRS}
)

# Define common libraries for all examples
set(GLEXAMPLES_DEPENDENCY_LIBRARIES
    ${GLBINDING_LIBRARIES}
    ${GLOBJECTS_LIBRARIES}
    ${GLOPERATE_LIBRARIES}
    ${LIBZEUG_LIBRARIES}
)

# Applications
set(IDE_FOLDER "")
add_subdirectory(emptyexample)
add_subdirectory(openglexample)
add_subdirectory(transparency)
add_subdirectory(glexamples-viewer)

# Tests
set(IDE_FOLDER "Tests")
add_subdirectory(tests)
