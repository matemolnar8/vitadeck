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
set(VITADECK_INCLUDE_DIRS vendor vendor/mujs-1.3.7)

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
    vendor/mujs-1.3.7/one.c
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
  list(APPEND VITADECK_LIBRARIES m)
endif()

target_link_libraries(${PROJECT_NAME} ${VITADECK_LIBRARIES})

add_custom_target(js_build
  COMMAND pnpm build
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/js
  COMMENT "Building JavaScript with pnpm"
  VERBATIM
)

add_custom_target(js_copy
  COMMENT "Copying js/dist to ${CMAKE_BINARY_DIR}/js"
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/js
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/js/dist ${CMAKE_BINARY_DIR}/js
  BYPRODUCTS ${CMAKE_BINARY_DIR}/js
)

add_custom_target(js
  DEPENDS js_build ${PROJECT_NAME}
  COMMENT "Build JavaScript and everything"
)

add_custom_target(cloc
  COMMENT "Count lines of code in ${CMAKE_SOURCE_DIR}"
  COMMAND cloc ${CMAKE_SOURCE_DIR} --match-d=src "--not-match-d=(out|node_modules)"
  VERBATIM
)

if(DEFINED ENV{VITASDK})
  set(VITA_ELF_CREATE_FLAGS "${VITA_ELF_CREATE_FLAGS}")

  add_dependencies(${PROJECT_NAME} js_copy)

  vita_create_self(eboot.bin ${PROJECT_NAME} UNSAFE NOSTRIP)

  vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} eboot.bin
      VERSION ${VITA_VERSION}
      NAME ${VITA_APP_NAME}
      FILE module module
      FILE js/dist/main.js js/main.js
  )

  # Create an explicit `upload_vpk` target to upload the VPK file to Vita's download folder
  add_custom_target(upload_vpk
    DEPENDS ${PROJECT_NAME}.vpk-vpk
    COMMAND echo "Sending ${PROJECT_NAME}.vpk to vita download folder"
    COMMAND curl -T ${PROJECT_NAME}.vpk ftp://${PSVITAIP}:1337/ux0:/download/
    COMMAND echo "VPK uploaded to ux0:/download/"
    COMMENT "Upload VPK file to Vita download folder"
    VERBATIM
  )

  # Create an explicit `upload_vita` target so uploads only run when requested.
  add_custom_target(upload_vita
    DEPENDS ${PROJECT_NAME}.vpk-vpk
    # Destroy running apps
    COMMAND echo "Destroy running apps"
    COMMAND echo destroy | nc ${PSVITAIP} 1338
    # Upload eboot.bin
    COMMAND echo "Sending ${VITA_TITLEID}/eboot.bin to vita"
    COMMAND curl -T eboot.bin ftp://${PSVITAIP}:1337/ux0:/app/${VITA_TITLEID}/
    # Upload js/main.js
    COMMAND echo "Sending ${VITA_TITLEID}/js/main.js to vita"
    COMMAND curl -T ${CMAKE_SOURCE_DIR}/js/dist/main.js ftp://${PSVITAIP}:1337/ux0:/app/${VITA_TITLEID}/js/main.js
    # Launch the app
    COMMAND echo "Launching ${VITA_TITLEID}"
    COMMAND echo launch ${VITA_TITLEID} | nc ${PSVITAIP} 1338
    COMMAND echo "Done"
    COMMENT "Upload built files to Vita and launch"
    VERBATIM
  )

  # Add a convenience target `run` that depends on upload_vita so `make run` will
  # build, upload, and launch the app on the Vita.
  add_custom_target(run
    DEPENDS upload_vita
    COMMENT "Build, upload, and launch on Vita"
    VERBATIM
  )
else()
  add_dependencies(${PROJECT_NAME} js_copy)
  add_custom_target(run
    DEPENDS ${PROJECT_NAME}
    VERBATIM
    COMMAND ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
  )
endif()