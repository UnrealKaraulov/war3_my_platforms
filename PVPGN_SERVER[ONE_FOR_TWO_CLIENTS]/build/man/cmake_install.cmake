# Install script for directory: C:/Projects/pvpgn-magic/source/man

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/pvpgn/share/man" TYPE FILE FILES
    "C:/Projects/pvpgn-magic/source/man/bnbot.1"
    "C:/Projects/pvpgn-magic/source/man/bnchat.1"
    "C:/Projects/pvpgn-magic/source/man/bnetd.1"
    "C:/Projects/pvpgn-magic/source/man/bnetd.conf.5"
    "C:/Projects/pvpgn-magic/source/man/bnftp.1"
    "C:/Projects/pvpgn-magic/source/man/bni2tga.1"
    "C:/Projects/pvpgn-magic/source/man/bnibuild.1"
    "C:/Projects/pvpgn-magic/source/man/bniextract.1"
    "C:/Projects/pvpgn-magic/source/man/bnilist.1"
    "C:/Projects/pvpgn-magic/source/man/bnpass.1"
    "C:/Projects/pvpgn-magic/source/man/bnpcap.1"
    "C:/Projects/pvpgn-magic/source/man/bnproxy.1"
    "C:/Projects/pvpgn-magic/source/man/bnstat.1"
    "C:/Projects/pvpgn-magic/source/man/bntext.5"
    "C:/Projects/pvpgn-magic/source/man/bntrackd.1"
    "C:/Projects/pvpgn-magic/source/man/tgainfo.1"
    )
endif()

