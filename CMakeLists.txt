cmake_minimum_required(VERSION 3.31.0)
project(cp_framework VERSION 0.1.0 LANGUAGES C CXX)

##########################################################
# LIB
##########################################################
add_library(${CMAKE_PROJECT_NAME} SHARED src/dllmain.cpp)

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