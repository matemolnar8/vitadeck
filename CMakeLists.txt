cmake_minimum_required(VERSION 3.10)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  endif()
endif()

project(vitadeck)

if(DEFINED ENV{VITASDK})
  message(STATUS "VITA is ON")
  include("${VITASDK}/share/vita.cmake" REQUIRED)
else()
  message(STATUS "VITA is OFF")
endif()

set(VITA_APP_NAME "VitaDeck")
set(PSVITAIP "192.168.1.191" CACHE STRING "PSVita IP (for FTP access)")
set(VITA_TITLEID  "PGRI00001")
set(VITA_VERSION  "01.00")
set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d PARENTAL_LEVEL=1")
set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d ATTRIBUTE2=12")

if(DEFINED ENV{VITASDK})
  # Find SDL2 (Static)
  find_library(SDL2_LIB SDL2 HINTS $ENV{VITASDK}/arm-vita-eabi/lib REQUIRED)
endif()

if(NOT DEFINED ENV{VITASDK})
  # Find raylib with pkg-config
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(RAYLIB REQUIRED raylib)
endif()

# Set up include directories
set(VITADECK_INCLUDE_DIRS mujs-1.3.7)

if(DEFINED ENV{VITASDK})
  list(APPEND VITADECK_INCLUDE_DIRS
    $ENV{VITASDK}/arm-vita-eabi/include
    $ENV{VITASDK}/arm-vita-eabi/include/SDL2
  )
else()
  list(APPEND VITADECK_INCLUDE_DIRS ${RAYLIB_INCLUDE_DIRS})
  list(APPEND VITADECK_LIBRARY_DIRS ${RAYLIB_LIBRARY_DIRS})
endif()

include_directories(${VITADECK_INCLUDE_DIRS})
link_directories(${VITADECK_LIBRARY_DIRS})

add_executable(${PROJECT_NAME}
    src/main.c
    mujs-1.3.7/one.c
)

set(VITADECK_LIBRARIES)

if(DEFINED ENV{VITASDK})
  # Set up link libraries (Correct Order)
  list(APPEND VITADECK_LIBRARIES
    raylib
    SDL2
    vitaGL
    vitashark
    SceShaccCgExt
    SceShaccCg_stub
    stdc++
    mathneon
    OpenSLES
    m
    c
    taihen_stub
    SceAppMgr_stub
    SceCtrl_stub
    SceKernelDmacMgr_stub
    SceGxm_stub
    SceCommonDialog_stub
    SceLibKernel_stub
    SceAudio_stub
    SceTouch_stub
    SceHid_stub
    SceMotion_stub
    SceSysmodule_stub
    SceIofilemgr_stub
    SceNetCtl_stub
    SceNet_stub
    SceDisplay_stub
    SceAppUtil_stub
    SceAudioIn_stub
    SceIofilemgr_stub
    ScePower_stub
    SceProcessmgr_stub
    SceIme_stub
  )
else()
  list(APPEND VITADECK_LIBRARIES ${RAYLIB_LIBRARIES})
endif()

target_link_libraries(${PROJECT_NAME} ${VITADECK_LIBRARIES})

if(DEFINED ENV{VITASDK})
  set(VITA_ELF_CREATE_FLAGS "${VITA_ELF_CREATE_FLAGS}")

  vita_create_self(eboot.bin ${PROJECT_NAME} UNSAFE NOSTRIP)

  vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} eboot.bin
      VERSION ${VITA_VERSION}
      NAME ${VITA_APP_NAME}
      FILE module module
      FILE src/js/main.js js/main.js
  )

  if(true)#change to true to send eboot.bin to vita [make sure to edit the ip of ps vita] (requires vitacompanion)
    add_custom_command(TARGET ${PROJECT_NAME}.vpk-vpk POST_BUILD
      COMMAND echo "Destroy running apps"
      COMMAND echo destroy | nc ${PSVITAIP} 1338
      # COMMAND echo "Sending ${PROJECT_NAME}.vpk to vita"
      # COMMAND curl -T ${PROJECT_NAME}.vpk ftp://${PSVITAIP}:1337/ux0:/download/
      COMMAND echo "Sending ${VITA_TITLEID}/eboot.bin to vita"
      COMMAND curl -T eboot.bin ftp://${PSVITAIP}:1337/ux0:/app/${VITA_TITLEID}/
      COMMAND echo "Sending ${VITA_TITLEID}/js/main.js to vita"
      COMMAND curl -T ../src/js/main.js ftp://${PSVITAIP}:1337/ux0:/app/${VITA_TITLEID}/js/main.js
      COMMAND echo "Launching ${VITA_TITLEID}"
      COMMAND echo launch ${VITA_TITLEID} | nc ${PSVITAIP} 1338
      COMMAND echo "Done"
    )
  endif()
else()
  # Copy js folder to build directory
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND cp -r ../src/js .
  )
endif()