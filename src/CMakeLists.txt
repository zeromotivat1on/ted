add_executable(${PROJECT_NAME}
                arena.h ted.h file.h font.h gl.h matrix.h memory.h memory_eater.h quat.h vector.h
                main.cpp ted.cpp file.cpp font.cpp gl.cpp matrix.cpp memory.cpp quat.cpp vector.cpp)

target_precompile_headers(${PROJECT_NAME} PUBLIC pch.h)
target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}")
target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/src")

if(MSVC)
    target_compile_options(${PROJECT_NAME} PUBLIC "/Zc:__cplusplus")
    target_compile_options(${PROJECT_NAME} PUBLIC "/W3")
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
                               # Platform
                               $<$<PLATFORM_ID:Windows>:WIN32>
                               # Configuration
                               $<$<CONFIG:Debug>:DEBUG>
                               $<$<CONFIG:Release>:RELEASE>
                               # Directories
                               DIR_SHADERS="${CMAKE_SOURCE_DIR}/shaders/")
