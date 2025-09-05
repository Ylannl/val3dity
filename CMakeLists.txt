if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
  cmake_policy(SET CMP0011 NEW)
endif()

cmake_minimum_required(VERSION 3.16)
project(val3dity VERSION 2.5.3)

option(VAL3DITY_LIBRARY "Build val3dity as a library instead of an executable." OFF)
option(VAL3DITY_USE_INTERNAL_DEPS "Use the thirdparty dir that ships with val3dity (for pugixml, nlohmann-json, spdlog and tclap). Turn off in case you want to provide these dependencies yourself." ON)
option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS "-O2")
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

if(MSVC)
  add_definitions("/EHsc")
endif()

# CGAL
find_package(CGAL QUIET COMPONENTS)
if(CGAL_FOUND)
  message(STATUS "CGAL found")
  message(STATUS ${CGAL_LIBRARIES})
  message(STATUS ${CGAL_3RD_PARTY_LIBRARIES})
else()
  message(SEND_ERROR "val3dity requires the CGAL library")
  return()
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Eigen
find_package(Eigen3 3.1.0 QUIET)
include(CGAL_Eigen3_support)

# Boost
find_package(Boost REQUIRED COMPONENTS filesystem)
if(NOT Boost_FOUND)
  message(SEND_ERROR "val3dity requires the Boost library")
  return()
endif()

# GEOS
find_package(GEOS CONFIG REQUIRED)
if(GEOS_FOUND)
  get_target_property(GEOS_LOCATION GEOS::geos_c LOCATION)
  message(STATUS "Found GEOS: ${GEOS_LOCATION}")
else()
  message(SEND_ERROR "val3dity requires the GEOS library")
endif()

# Handle internal vs external dependencies
if(VAL3DITY_USE_INTERNAL_DEPS)
  message(STATUS "Using internal dependencies")
  file(GLOB_RECURSE THIRDPARTY ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/*.cpp)
  add_library(val3dity_thirdparty STATIC ${THIRDPARTY})
  target_include_directories(val3dity_thirdparty PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/pugixml
  )
  set_property(TARGET val3dity_thirdparty PROPERTY POSITION_INDEPENDENT_CODE ON)
else()
  message(STATUS "Using external dependencies")
  find_package(nlohmann_json CONFIG REQUIRED)
  find_package(spdlog CONFIG REQUIRED)
  find_package(pugixml CONFIG REQUIRED)

  # Find TCLAP's include path (TCLAP does not provide a CMake target)
  find_path(TCLAP_INCLUDE_DIR "tclap/CmdLine.h")
  if(TCLAP_INCLUDE_DIR)
    message(STATUS "Found TCLAP: ${TCLAP_INCLUDE_DIR}")
  else()
    message(FATAL_ERROR "TCLAP not found. Make sure it is installed via eg. vcpkg.")
  endif()
endif()

# Create an INTERFACE target to consolidate dependency propagation
add_library(val3dity_deps INTERFACE)
target_link_libraries(val3dity_deps INTERFACE
  CGAL::CGAL
  CGAL::Eigen3_support
  GEOS::geos_c
)
if(VAL3DITY_USE_INTERNAL_DEPS)
  target_link_libraries(val3dity_deps INTERFACE val3dity_thirdparty)
else()
  target_link_libraries(val3dity_deps INTERFACE
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    pugixml::pugixml
  )
endif()

# Create an object library for shared sources
file(GLOB SRC_FILES src/*.cpp)
list(REMOVE_ITEM SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
list(REMOVE_ITEM SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/val3dity.cpp)

add_library(val3dity_core OBJECT ${SRC_FILES})
target_compile_features(val3dity_core PUBLIC cxx_std_17)
target_link_libraries(val3dity_core val3dity_deps)

if(VAL3DITY_LIBRARY)
  message(STATUS "Building val3dity library")

  add_library(val3dity
    ${CMAKE_CURRENT_SOURCE_DIR}/src/val3dity.cpp
    $<TARGET_OBJECTS:val3dity_core>
  )
  target_compile_features(val3dity PUBLIC cxx_std_17)
  target_include_directories(val3dity PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
  )
  target_link_libraries(val3dity PUBLIC val3dity_deps)
  set_property(TARGET val3dity PROPERTY POSITION_INDEPENDENT_CODE ON)

  # Install headers
  file(GLOB VAL3DITY_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
  install(FILES ${VAL3DITY_HEADERS} DESTINATION include/val3dity)

  # Create and install CMake package configuration
  include(CMakePackageConfigHelpers)
  include(GNUInstallDirs)

  write_basic_package_version_file(
      "val3dity-config-version.cmake"
      COMPATIBILITY AnyNewerVersion
  )

  export(TARGETS val3dity val3dity_deps
      FILE "${CMAKE_CURRENT_BINARY_DIR}/val3dity-targets.cmake"
  )

  configure_package_config_file(
      "val3dity-config.cmake.in"
      "${CMAKE_CURRENT_BINARY_DIR}/val3dity-config.cmake"
      INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/val3dity"
  )

  install(
      FILES
          "${CMAKE_CURRENT_BINARY_DIR}/val3dity-config.cmake"
          "${CMAKE_CURRENT_BINARY_DIR}/val3dity-config-version.cmake"
      DESTINATION
          "${CMAKE_INSTALL_LIBDIR}/cmake/val3dity"
  )

  install(
      TARGETS val3dity val3dity_deps
      EXPORT val3dity-targets
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )

  install(EXPORT val3dity-targets
    FILE val3dity-targets.cmake
    NAMESPACE val3dity::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/val3dity
  )

else()
  message(STATUS "Building val3dity executable")

  add_executable(val3dity ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
  target_compile_features(val3dity PRIVATE cxx_std_17)
  target_link_libraries(val3dity PRIVATE val3dity_core Boost::filesystem)
  install(TARGETS val3dity DESTINATION bin)

  if(NOT VAL3DITY_USE_INTERNAL_DEPS)
    target_include_directories(val3dity PRIVATE ${TCLAP_INCLUDE_DIR})
  endif()
endif()
