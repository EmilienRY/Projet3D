cmake_minimum_required(VERSION 3.16)

project(RayTracingGPU VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Trouver Qt6 (Qt6.10)
find_package(Qt6 REQUIRED COMPONENTS Gui OpenGLWidgets)
find_package(Qt6 REQUIRED COMPONENTS Concurrent)

qt_standard_project_setup(REQUIRES 6.8)

# --- Crée l’exécutable principal AVANT de définir les sources
qt_add_executable(appRayTracingGPU
    src/renderer/openglwindow.h
    src/mainwindow.h src/mainwindow.cpp)

# --- Définir les fichiers source
target_sources(appRayTracingGPU PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/openglwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/camera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/scene/scene.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/scene/mesh.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/scene/material.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/scene/light.cpp
)

# --- Inclure les headers
target_include_directories(appRayTracingGPU PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# --- Lier Qt
target_link_libraries(appRayTracingGPU
    PRIVATE Qt6::Gui Qt6::OpenGLWidgets
    PRIVATE Qt6::Concurrent
)

# --- Installation (optionnelle)
install(TARGETS appRayTracingGPU
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# --- Copier les shaders dans le dossier de build à la compilation
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/src)

