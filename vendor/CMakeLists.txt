target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/vendor")

# Add glad
target_include_directories(${PROJECT_NAME} PRIVATE glad/include)
target_sources(${PROJECT_NAME} PRIVATE glad/src/glad.cpp)

# Add glfw
target_include_directories(${PROJECT_NAME} PRIVATE glfw/include)
target_link_libraries(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/vendor/glfw/lib-vc2022/glfw3.lib")
