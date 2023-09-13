# Install script for directory: C:/Projects/pvpgn-magic/source/conf/i18n

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/pvpgn/conf/i18n" TYPE FILE FILES
    "C:/Projects/pvpgn-magic/source/conf/i18n/bnhelp.conf"
    "C:/Projects/pvpgn-magic/source/conf/i18n/bnmotd.txt"
    "C:/Projects/pvpgn-magic/source/conf/i18n/chathelp-war3.txt"
    "C:/Projects/pvpgn-magic/source/conf/i18n/common.xml"
    "C:/Projects/pvpgn-magic/source/conf/i18n/newaccount.txt"
    "C:/Projects/pvpgn-magic/source/conf/i18n/news.txt"
    "C:/Projects/pvpgn-magic/source/conf/i18n/termsofservice.txt"
    "C:/Projects/pvpgn-magic/source/conf/i18n/w3motd.txt"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/pvpgn/conf/i18n" TYPE DIRECTORY FILES
    "C:/Projects/pvpgn-magic/source/conf/i18n/bgBG"
    "C:/Projects/pvpgn-magic/source/conf/i18n/csCZ"
    "C:/Projects/pvpgn-magic/source/conf/i18n/deDE"
    "C:/Projects/pvpgn-magic/source/conf/i18n/esES"
    "C:/Projects/pvpgn-magic/source/conf/i18n/frFR"
    "C:/Projects/pvpgn-magic/source/conf/i18n/itIT"
    "C:/Projects/pvpgn-magic/source/conf/i18n/jpJA"
    "C:/Projects/pvpgn-magic/source/conf/i18n/koKR"
    "C:/Projects/pvpgn-magic/source/conf/i18n/nlNL"
    "C:/Projects/pvpgn-magic/source/conf/i18n/plPL"
    "C:/Projects/pvpgn-magic/source/conf/i18n/ptBR"
    "C:/Projects/pvpgn-magic/source/conf/i18n/ruRU"
    "C:/Projects/pvpgn-magic/source/conf/i18n/svSE"
    "C:/Projects/pvpgn-magic/source/conf/i18n/zhCN"
    "C:/Projects/pvpgn-magic/source/conf/i18n/zhTW"
    )
endif()

