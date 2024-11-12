# Include directories for CustomEnv header files
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../contrib/ai/examples/simple/sim)

# Define the CustomEnv library if not already defined
add_library(CustomEnvLib SHARED ../contrib/ai/examples/simple/sim/CustomEnv.cpp)

# Link CustomEnvLib with necessary NS-3 libraries
target_link_libraries(CustomEnvLib
    ${ns3-libs}
    ${ns3-contrib-libs}
)

# Function to create scratch targets
set(target_prefix scratch_)

function(create_scratch source_files)
  # Return early if no sources in the subdirectory
  list(LENGTH source_files number_sources)
  if(number_sources EQUAL 0)
    return()
  endif()

  # Find the main function in the source files
  set(scratch_src)
  foreach(source_file ${source_files})
    file(READ ${source_file} source_file_contents)
    string(REGEX MATCHALL "main[(| (]" main_position "${source_file_contents}")
    if(CMAKE_MATCH_0)
      list(APPEND scratch_src ${source_file})
    endif()
  endforeach()

  list(LENGTH scratch_src scratch_src_len)

  # Raise errors if there are issues with the main function
  if(${scratch_src_len} EQUAL 0)
    message(FATAL_ERROR "The following scratch source files do not contain a main function: ${source_files}")
  endif()
  if(${scratch_src_len} GREATER 1)
    message(FATAL_ERROR "Multiple files contain a main function: ${scratch_src}")
  endif()

  # Get parent directory name and source name
  get_filename_component(scratch_dirname ${scratch_src} DIRECTORY)
  string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" scratch_dirname "${scratch_dirname}")
  string(REPLACE "/" "_" scratch_dirname "${scratch_dirname}")

  if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.14.0")
    get_filename_component(scratch_name ${scratch_src} NAME_WLE)
  else()
    get_filename_component(scratch_name ${scratch_src} NAME)
    string(FIND "${scratch_name}" "." ext_position REVERSE)
    if(${ext_position} EQUAL -1)
      message(FATAL_ERROR "Source file has no extension: ${scratch_src}")
    else()
      string(SUBSTRING "${scratch_name}" 0 ${ext_position} scratch_name)
    endif()
  endif()

  set(target_prefix scratch_)
  if(scratch_dirname)
    set(target_prefix scratch${scratch_dirname}_)
  endif()

  get_filename_component(scratch_src ${scratch_src} ABSOLUTE)
  get_filename_component(scratch_absolute_directory ${scratch_src} DIRECTORY)
  string(REPLACE "${PROJECT_SOURCE_DIR}" "${CMAKE_OUTPUT_DIRECTORY}" scratch_directory ${scratch_absolute_directory})

  # Build the executable and link with CustomEnvLib and NS-3 libraries
  build_exec(
    EXECNAME ${scratch_name}
    EXECNAME_PREFIX ${target_prefix}
    SOURCE_FILES "${source_files}"
    LIBRARIES_TO_LINK "${ns3-libs}" "${ns3-contrib-libs}" CustomEnvLib
    EXECUTABLE_DIRECTORY_PATH ${scratch_directory}/
  )
endfunction()

# Scan *.cc files in ns-3-dev/scratch and build a target for each
file(GLOB single_source_file_scratches CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/[^.]*.cc)
foreach(scratch_src ${single_source_file_scratches})
  create_scratch(${scratch_src})
endforeach()

# Scan ns-3-dev/scratch subdirectories
file(
  GLOB scratch_subdirectories
  CONFIGURE_DEPENDS
  LIST_DIRECTORIES true
  ${CMAKE_CURRENT_SOURCE_DIR}/**
)
foreach(entry ${scratch_subdirectories})
  if(NOT (IS_DIRECTORY ${entry}))
    list(REMOVE_ITEM scratch_subdirectories ${entry})
  endif()
endforeach()

# Build scratches per directory or following CMakeLists.txt instructions
foreach(subdir ${scratch_subdirectories})
  if(EXISTS ${subdir}/CMakeLists.txt)
    # If the subdirectory contains a CMakeLists.txt file, use it
    add_subdirectory(${subdir})
  else()
    # Otherwise, build all source files in the directory as a scratch
    file(GLOB scratch_sources CONFIGURE_DEPENDS ${subdir}/[^.]*.cc)
    create_scratch("${scratch_sources}")
  endif()
endforeach()
