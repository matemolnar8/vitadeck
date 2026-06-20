cmake_minimum_required(VERSION 3.10)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  endif()
endif()

project(vitadeck)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(DEFINED ENV{VITASDK})
  message(STATUS "VITA is ON")
  include("${VITASDK}/share/vita.cmake" REQUIRED)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
else()
  message(STATUS "VITA is OFF")
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Og -g")
  else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
  endif()
endif()

set(VITA_APP_NAME "VitaDeck")
set(PSVITAIP "192.168.1.177" CACHE STRING "PSVita IP (for FTP access)")
set(VITA_TITLEID  "PGRI00001")
set(VITA_VERSION  "01.00")
set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d PARENTAL_LEVEL=1")
set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d ATTRIBUTE2=12")

if(DEFINED ENV{VITASDK})
  find_library(SDL2_LIB SDL2 HINTS $ENV{VITASDK}/arm-vita-eabi/lib REQUIRED)
endif()

if(NOT DEFINED ENV{VITASDK})
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(RAYLIB REQUIRED raylib)
endif()

find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
if(NOT DEFINED ENV{VITASDK})
  find_package(ZLIB REQUIRED)
endif()

# Set up include directories
set(VITADECK_INCLUDE_DIRS src vendor vendor/quickjs)
list(APPEND VITADECK_INCLUDE_DIRS ${OPENSSL_INCLUDE_DIR})

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
# System libcurl (e.g. macOS SDK) lives under usr/include; -I there makes Clang
# diagnose SDK headers. SYSTEM uses -isystem so libc headers stay quiet.
include_directories(SYSTEM ${CURL_INCLUDE_DIRS})
link_directories(${VITADECK_LIBRARY_DIRS})

# QuickJS
set(QUICKJS_SOURCES
    vendor/quickjs/quickjs.c
    vendor/quickjs/libregexp.c
    vendor/quickjs/libunicode.c
    vendor/quickjs/cutils.c
    vendor/quickjs/dtoa.c
)
add_library(quickjs STATIC ${QUICKJS_SOURCES})
target_compile_definitions(quickjs PRIVATE
    CONFIG_VERSION="2024-01-13"
    $<$<NOT:$<BOOL:$ENV{VITASDK}>>:_GNU_SOURCE>
)
if(DEFINED ENV{VITASDK})
    target_compile_definitions(quickjs PRIVATE __vita__)
endif()

set(PLATFORM_SOURCES)
if(DEFINED ENV{VITASDK})
  list(APPEND PLATFORM_SOURCES src/platform/thread_vita.c)
else()
  list(APPEND PLATFORM_SOURCES src/platform/thread_posix.c)
endif()

set(JSLIB_SOURCES
    src/jslib/jslib.c
    src/jslib/instance.c
    src/jslib/colors.c
    src/jslib/timeout.c
    src/jslib/log.c
    src/jslib/fetch.c
)

set(VITADECK_SOURCES
    src/main.c
    src/core/arena.c
    src/core/bootstrap.c
    src/core/js_runtime.c
    src/core/package_library.c
    ${JSLIB_SOURCES}
    src/ui/instance_tree.c
    src/ui/fonts.c
    src/ui/render.c
    src/ui/input.c
    src/ui/scroll.c
    src/core/event_queue.c
    src/shell/shell.c
    src/upload/archive.c
    src/upload/http_server.c
    ${PLATFORM_SOURCES}
)
add_executable(${PROJECT_NAME} ${VITADECK_SOURCES})

set(VITADECK_LIBRARIES m quickjs)

if(DEFINED ENV{VITASDK})
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
    SceHttp_stub
    SceSsl_stub
    SceDisplay_stub
    SceAppUtil_stub
    SceAudioIn_stub
    ScePower_stub
    SceProcessmgr_stub
    SceIme_stub
    ${CURL_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    z
  )
else()
  list(APPEND VITADECK_LIBRARIES ${RAYLIB_LIBRARIES} ${CURL_LIBRARIES} ${OPENSSL_LIBRARIES})
  list(APPEND VITADECK_LIBRARIES ZLIB::ZLIB)
  list(APPEND VITADECK_LIBRARIES pthread)
endif()

target_link_libraries(${PROJECT_NAME} ${VITADECK_LIBRARIES})

if(NOT DEFINED ENV{VITASDK})
  include(CTest)
  if(BUILD_TESTING)
    add_executable(timer_reload_harness tests/timer_reload_harness.c src/jslib/timeout.c)
    target_link_libraries(timer_reload_harness quickjs m ${RAYLIB_LIBRARIES})
    add_test(NAME timer_reload_harness COMMAND timer_reload_harness)

    set(SMOKE_HARNESS_SOURCES
      tests/smoke_harness.c
      src/core/arena.c
      src/core/bootstrap.c
      src/core/js_runtime.c
      src/core/package_library.c
      src/core/event_queue.c
      ${JSLIB_SOURCES}
      src/ui/instance_tree.c
      src/ui/fonts.c
      src/ui/render.c
      src/ui/input.c
      src/ui/scroll.c
      src/platform/thread_posix.c
    )
    add_executable(smoke_harness ${SMOKE_HARNESS_SOURCES})
    target_link_libraries(smoke_harness ${VITADECK_LIBRARIES})
    add_dependencies(smoke_harness js_copy)
    add_test(
      NAME smoke_harness
      COMMAND ${CMAKE_SOURCE_DIR}/tests/run_smoke_test.sh ${CMAKE_BINARY_DIR}
    )
    set_tests_properties(smoke_harness PROPERTIES
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      LABELS "smoke"
    )
  endif()
endif()

add_custom_target(js_build
  COMMAND pnpm build:prod
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/js
  COMMENT "Building JavaScript with pnpm"
  VERBATIM
)

add_custom_target(js_copy
  COMMENT "Copying js/dist/runtime to ${CMAKE_BINARY_DIR}/js"
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/js/dist/runtime ${CMAKE_BINARY_DIR}/js
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/assets
  BYPRODUCTS ${CMAKE_BINARY_DIR}/js
)

add_custom_target(js
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/js
  COMMAND pnpm build:prod
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ${PROJECT_NAME}
  COMMENT "Build JS bundle (pnpm build:prod) then native ${PROJECT_NAME}"
  VERBATIM
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
      FILE js/dist/runtime/runtime.js js/runtime.js
      FILE assets/fonts/DejaVuSans.ttf assets/fonts/DejaVuSans.ttf
      FILE assets/fonts/DejaVuSans-LICENSE.txt assets/fonts/DejaVuSans-LICENSE.txt
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
    # Upload runtime and active Deck App package
    COMMAND echo "Sending ${VITA_TITLEID}/js/runtime.js to vita"
    COMMAND curl -T ${CMAKE_SOURCE_DIR}/js/dist/runtime/runtime.js ftp://${PSVITAIP}:1337/ux0:/app/${VITA_TITLEID}/js/runtime.js
    COMMAND echo "Sending ${VITA_TITLEID}/assets/fonts/DejaVuSans.ttf to vita"
    COMMAND curl --ftp-create-dirs -T ${CMAKE_SOURCE_DIR}/assets/fonts/DejaVuSans.ttf ftp://${PSVITAIP}:1337/ux0:/app/${VITA_TITLEID}/assets/fonts/DejaVuSans.ttf
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