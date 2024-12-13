add_executable(ted
                file_reader.h font.h
                main.cpp file_reader.cpp font.cpp)
                
target_precompile_headers(ted PUBLIC pch.h)

if (WIN32)
   target_sources(ted PRIVATE)
endif()

if(MSVC)
    target_compile_options(ted PUBLIC "/Zc:__cplusplus")
    target_compile_options(ted PUBLIC "/W3")
endif()

target_compile_definitions(ted PRIVATE
                               # Platform
                               $<$<PLATFORM_ID:Windows>:WIN32>
                               # Configuration
                               $<$<CONFIG:Debug>:BUILD_DEBUG>
                               $<$<CONFIG:Release>:BUILD_RELEASE>)

# Add GDL
add_subdirectory("${PROJECT_SOURCE_DIR}/src/gdl")
target_link_libraries(ted PRIVATE gdl)
