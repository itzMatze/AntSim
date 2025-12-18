cmake_minimum_required(VERSION 3.19)
project(AntSim)
set(CMAKE_CXX_STANDARD 20)

set(GLM_DIR "${PROJECT_SOURCE_DIR}/dependencies/glm")
set(VKTE_DIR "${PROJECT_SOURCE_DIR}/dependencies/vkTurboEngine")

set(ENABLE_VKTE_LOGGING ON)
add_subdirectory(${VKTE_DIR})

set(SOURCE_FILES
	src/ants.cpp
	src/application.cpp
	src/hash_grid.cpp
	src/work_context.cpp
	src/util/random_generator.cpp
	src/util/color.cpp
)

add_executable(AntSim src/main.cpp ${SOURCE_FILES})
target_include_directories(AntSim PRIVATE "${PROJECT_SOURCE_DIR}/include"
	"${PROJECT_SOURCE_DIR}/dependencies/"
	"${GLM_DIR}/"
	"${VKTE_DIR}/include/"
)
target_link_libraries(AntSim PRIVATE vkte::window)
if (CMAKE_IMPORT_LIBRARY_SUFFIX)
	add_custom_command(TARGET AntSim POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		$<TARGET_RUNTIME_DLLS:AntSim>
		$<TARGET_FILE_DIR:AntSim>
		COMMAND_EXPAND_LISTS
	)
endif()
