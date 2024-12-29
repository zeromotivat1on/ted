target_include_directories(ted PRIVATE "${PROJECT_SOURCE_DIR}/vendor")

# Add glad
target_include_directories(ted PRIVATE glad/include)
target_sources(ted PRIVATE glad/src/glad.cpp)

# Add glfw
target_include_directories(ted PRIVATE glfw/include)
target_link_libraries(ted PRIVATE "${PROJECT_SOURCE_DIR}/vendor/glfw/lib-vc2022/glfw3.lib")
