# Install script for directory: C:/Projects/pvpgn-magic/source/files

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

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/pvpgn/var" TYPE DIRECTORY FILES "C:/Projects/pvpgn-magic/build/files/var/")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/pvpgn/var/files" TYPE FILE FILES
    "C:/Projects/pvpgn-magic/source/files/ad000001.png"
    "C:/Projects/pvpgn-magic/source/files/ad000001.smk"
    "C:/Projects/pvpgn-magic/source/files/ad000002.mng"
    "C:/Projects/pvpgn-magic/source/files/newbie.save"
    "C:/Projects/pvpgn-magic/source/files/bnserver.ini"
    "C:/Projects/pvpgn-magic/source/files/bnserver-D2DV.ini"
    "C:/Projects/pvpgn-magic/source/files/bnserver-D2XP.ini"
    "C:/Projects/pvpgn-magic/source/files/bnserver-WAR3.ini"
    "C:/Projects/pvpgn-magic/source/files/ver-IX86-1.mpq"
    "C:/Projects/pvpgn-magic/source/files/IX86ver1.mpq"
    "C:/Projects/pvpgn-magic/source/files/PMACver1.mpq"
    "C:/Projects/pvpgn-magic/source/files/XMACver1.mpq"
    "C:/Projects/pvpgn-magic/source/files/IX86ExtraWork.mpq"
    "C:/Projects/pvpgn-magic/source/files/icons.bni"
    "C:/Projects/pvpgn-magic/source/files/icons-WAR3.bni"
    )
endif()

