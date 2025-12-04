if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
endif()

cmake_minimum_required(VERSION 3.31.0)
project(cp_framework VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

##########################################################
# LIB
##########################################################
add_library(${CMAKE_PROJECT_NAME} SHARED 
    src/dllmain.cpp
    src/core/debug.cpp 
    src/core/filesystem.cpp
    src/core/algorithm.cpp
    src/core/compression.cpp
    src/core/threadPool.cpp
    src/core/serializable.cpp
    src/core/security.cpp
    src/core/types.cpp
    src/core/export.cpp
    src/core/math.cpp
    src/core/events.cpp
    src/core/delegate.cpp
    src/core/hybridEvents.cpp
    src/core/diagnostics.cpp

    src/graphics/vulkan/vkTypes.cpp
)

##########################################################
# CTEST
##########################################################
include(CTest)
enable_testing()

##########################################################
# CPACK
##########################################################
set(CPACK_PROJECT_NAME ${CMAKE_PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${CMAKE_PROJECT_VERSION})
include(CPack)

##########################################################
# PACKAGES
##########################################################
find_package(fmt CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(MbedTLS CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Vulkan)
find_package(VulkanHeaders CONFIG)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

##########################################################
# TARGET
##########################################################
target_include_directories(${CMAKE_PROJECT_NAME} 
    PUBLIC
        include
    PRIVATE
        ${Stb_INCLUDE_DIR}
)

target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE
        MbedTLS::mbedtls
        nlohmann_json::nlohmann_json
        Vulkan::Vulkan Vulkan::Headers GPUOpen::VulkanMemoryAllocator
        ZLIB::ZLIB
        EnTT::EnTT
        glfw
        imgui::imgui
    PUBLIC
        fmt::fmt
        glm::glm
)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE CP_FRAMEWORK_EXPORTS)

##########################################################
# DOXYGEN
##########################################################
find_package(Doxygen)
if(DOXYGEN_FOUND)
    # Configure Doxyfile from template to place output in the build tree
    set(DOXYFILE_IN ${CMAKE_SOURCE_DIR}/Doxyfile.in)
    set(DOXYFILE_OUT ${CMAKE_BINARY_DIR}/Doxyfile)

    configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

    add_custom_target(doc_doxygen
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)
else()
    message(STATUS "Doxygen not found. Install Doxygen to enable 'doc_doxygen' target.")
endif()

##########################################################
# PLANTUML (UML diagrams)
##########################################################
find_program(PLANTUML_EXECUTABLE NAMES plantuml)
set(PLANTUML_JAR "${CMAKE_SOURCE_DIR}/tools/plantuml/plantuml.jar")

if(PLANTUML_EXECUTABLE)
    add_custom_target(doc_uml
        COMMAND ${PLANTUML_EXECUTABLE} -tpng -o ${CMAKE_BINARY_DIR}/docs/uml ${CMAKE_SOURCE_DIR}/docs/uml/*.puml
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating UML diagrams with PlantUML"
        VERBATIM)
elseif(EXISTS ${PLANTUML_JAR})
    find_program(JAVA_EXECUTABLE NAMES java)
    if(JAVA_EXECUTABLE)
        add_custom_target(doc_uml
            COMMAND ${JAVA_EXECUTABLE} -jar ${PLANTUML_JAR} -tpng -o ${CMAKE_BINARY_DIR}/docs/uml ${CMAKE_SOURCE_DIR}/docs/uml/*.puml
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Generating UML diagrams with PlantUML (using local plantuml.jar)"
            VERBATIM)
    else()
        message(STATUS "PlantUML jar found but Java not available. Install Java to enable 'doc_uml' target or install PlantUML.")
    endif()
else()
    message(STATUS "PlantUML not found. Install PlantUML in PATH.")
endif()