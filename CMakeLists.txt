
cmake_minimum_required(VERSION 2.8)

# enable Qt AutoMoc
set(CMAKE_AUTOMOC ON)

# confirm that Qt5 is installed
find_package(Qt5 COMPONENTS Widgets)

if(NOT DEFINED Qt5Widgets_FOUND) 
	message("-- camera-capture:  Qt5Widgets package not found, skipping...")
	message("-- camera-capture:  run 'sudo apt-get install qtbase5-dev'")
	return()
endif()

message("-- found Qt5Widgets version:  ${Qt5Widgets_VERSION}")
message("-- found Qt5Widgets defines:  ${Qt5Widgets_DEFINITIONS}") 
message("-- found Qt5Widgets library:  ${Qt5Widgets_LIBRARIES}") 
message("-- found Qt5Widgets include:  ${Qt5Widgets_INCLUDE_DIRS}")


# determine if camera-collection is being built as a submodule inside another repo,
# or if it's being build standalone (if the later, we need to do some configuration)
get_directory_property(hasParent PARENT_DIRECTORY)

if(hasParent)
	message("-- camera-capture:  building as submodule of ${hasParent}")
else()
	message("-- camera-capture:  building as standalone")
	
	# standalone project
	project(camera-collection)
  
	# -std=gnu++11
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-write-strings")	
  
	# setup project output paths
	set(PROJECT_OUTPUT_DIR  ${PROJECT_BINARY_DIR}/${CMAKE_SYSTEM_PROCESSOR})
	set(PROJECT_INCLUDE_DIR ${PROJECT_OUTPUT_DIR}/include)

	file(MAKE_DIRECTORY ${PROJECT_INCLUDE_DIR})
	file(MAKE_DIRECTORY ${PROJECT_OUTPUT_DIR}/bin)

	message("-- system arch:  ${CMAKE_SYSTEM_PROCESSOR}")
	message("-- output path:  ${PROJECT_OUTPUT_DIR}")

	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_OUTPUT_DIR}/bin)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_OUTPUT_DIR}/lib)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_OUTPUT_DIR}/lib)
	
	include_directories(${PROJECT_INCLUDE_DIR})

	find_package(jetson-utils)
	find_package(jetson-inference)
endif()


#
# build tool
#
file(GLOB cameraCaptureSources *.cpp)

cuda_add_executable(camera-capture ${cameraCaptureSources})

target_link_libraries(camera-capture jetson-inference jetson-utils Qt5::Widgets)	

install(TARGETS camera-capture DESTINATION bin)


