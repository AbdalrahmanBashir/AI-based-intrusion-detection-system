# CMAKE generated file: DO NOT EDIT!
# Generated by CMake Version 3.22
cmake_policy(SET CMP0009 NEW)

# scratch_subdirectories at scratch/CMakeLists.txt:86 (file)
file(GLOB NEW_GLOB LIST_DIRECTORIES true "/home/abdohomad/workspace/ns-allinone-3.42/ns-3.42/scratch/**")
set(OLD_GLOB
  "/home/abdohomad/workspace/ns-allinone-3.42/ns-3.42/scratch/CMakeLists.txt"
  "/home/abdohomad/workspace/ns-allinone-3.42/ns-3.42/scratch/nested-subdir"
  "/home/abdohomad/workspace/ns-allinone-3.42/ns-3.42/scratch/scratch-simulator.cc"
  "/home/abdohomad/workspace/ns-allinone-3.42/ns-3.42/scratch/subdir"
  "/home/abdohomad/workspace/ns-allinone-3.42/ns-3.42/scratch/test1"
  "/home/abdohomad/workspace/ns-allinone-3.42/ns-3.42/scratch/uav-network"
  )
if(NOT "${NEW_GLOB}" STREQUAL "${OLD_GLOB}")
  message("-- GLOB mismatch!")
  file(TOUCH_NOCREATE "/home/abdohomad/workspace/ns-allinone-3.42/ns-3.42/cmake-cache/CMakeFiles/cmake.verify_globs")
endif()

# single_source_file_scratches at scratch/CMakeLists.txt:80 (file)
file(GLOB NEW_GLOB LIST_DIRECTORIES true "/home/abdohomad/workspace/ns-allinone-3.42/ns-3.42/scratch/[^.]*.cc")
set(OLD_GLOB
  "/home/abdohomad/workspace/ns-allinone-3.42/ns-3.42/scratch/scratch-simulator.cc"
  )
if(NOT "${NEW_GLOB}" STREQUAL "${OLD_GLOB}")
  message("-- GLOB mismatch!")
  file(TOUCH_NOCREATE "/home/abdohomad/workspace/ns-allinone-3.42/ns-3.42/cmake-cache/CMakeFiles/cmake.verify_globs")
endif()

# scratch_sources at scratch/CMakeLists.txt:105 (file)
file(GLOB NEW_GLOB LIST_DIRECTORIES true "/home/abdohomad/workspace/ns-allinone-3.42/ns-3.42/scratch/subdir/[^.]*.cc")
set(OLD_GLOB
  "/home/abdohomad/workspace/ns-allinone-3.42/ns-3.42/scratch/subdir/scratch-subdir-additional-header.cc"
  "/home/abdohomad/workspace/ns-allinone-3.42/ns-3.42/scratch/subdir/scratch-subdir.cc"
  )
if(NOT "${NEW_GLOB}" STREQUAL "${OLD_GLOB}")
  message("-- GLOB mismatch!")
  file(TOUCH_NOCREATE "/home/abdohomad/workspace/ns-allinone-3.42/ns-3.42/cmake-cache/CMakeFiles/cmake.verify_globs")
endif()

# scratch_sources at scratch/CMakeLists.txt:105 (file)
file(GLOB NEW_GLOB LIST_DIRECTORIES true "/home/abdohomad/workspace/ns-allinone-3.42/ns-3.42/scratch/test1/[^.]*.cc")
set(OLD_GLOB
  )
if(NOT "${NEW_GLOB}" STREQUAL "${OLD_GLOB}")
  message("-- GLOB mismatch!")
  file(TOUCH_NOCREATE "/home/abdohomad/workspace/ns-allinone-3.42/ns-3.42/cmake-cache/CMakeFiles/cmake.verify_globs")
endif()

# scratch_sources at scratch/CMakeLists.txt:105 (file)
file(GLOB NEW_GLOB LIST_DIRECTORIES true "/home/abdohomad/workspace/ns-allinone-3.42/ns-3.42/scratch/uav-network/[^.]*.cc")
set(OLD_GLOB
  "/home/abdohomad/workspace/ns-allinone-3.42/ns-3.42/scratch/uav-network/fanet-udp-app.cc"
  "/home/abdohomad/workspace/ns-allinone-3.42/ns-3.42/scratch/uav-network/flocking-mobility-model.cc"
  "/home/abdohomad/workspace/ns-allinone-3.42/ns-3.42/scratch/uav-network/flooding-attack-app.cc"
  "/home/abdohomad/workspace/ns-allinone-3.42/ns-3.42/scratch/uav-network/impersonation-attack-app.cc"
  "/home/abdohomad/workspace/ns-allinone-3.42/ns-3.42/scratch/uav-network/main.cc"
  "/home/abdohomad/workspace/ns-allinone-3.42/ns-3.42/scratch/uav-network/malicious-tag.cc"
  "/home/abdohomad/workspace/ns-allinone-3.42/ns-3.42/scratch/uav-network/mitm-app.cc"
  "/home/abdohomad/workspace/ns-allinone-3.42/ns-3.42/scratch/uav-network/sybil-attack-app.cc"
  )
if(NOT "${NEW_GLOB}" STREQUAL "${OLD_GLOB}")
  message("-- GLOB mismatch!")
  file(TOUCH_NOCREATE "/home/abdohomad/workspace/ns-allinone-3.42/ns-3.42/cmake-cache/CMakeFiles/cmake.verify_globs")
endif()
