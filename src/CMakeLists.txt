add_executable(${PROJECT_NAME}
                arena.h file.h font.h gap_buffer.h gl.h matrix.h memory.h memory_eater.h ted.h vector.h
                main.cpp file.cpp font.cpp gap_buffer.cpp gl.cpp matrix.cpp memory.cpp ted.cpp vector.cpp)

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
