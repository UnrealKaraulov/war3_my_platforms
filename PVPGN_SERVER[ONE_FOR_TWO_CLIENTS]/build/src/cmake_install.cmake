# Install script for directory: C:/Projects/pvpgn-magic/source/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "pvpgn")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("C:/Projects/pvpgn-magic/build/src/compat/cmake_install.cmake")
  include("C:/Projects/pvpgn-magic/build/src/common/cmake_install.cmake")
  include("C:/Projects/pvpgn-magic/build/src/win32/cmake_install.cmake")
  include("C:/Projects/pvpgn-magic/build/src/tinycdb/cmake_install.cmake")
  include("C:/Projects/pvpgn-magic/build/src/bntrackd/cmake_install.cmake")
  include("C:/Projects/pvpgn-magic/build/src/client/cmake_install.cmake")
  include("C:/Projects/pvpgn-magic/build/src/bniutils/cmake_install.cmake")
  include("C:/Projects/pvpgn-magic/build/src/bnpass/cmake_install.cmake")
  include("C:/Projects/pvpgn-magic/build/src/bnetd/cmake_install.cmake")
  include("C:/Projects/pvpgn-magic/build/src/test/cmake_install.cmake")

endif()

