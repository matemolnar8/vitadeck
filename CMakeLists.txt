cmake_minimum_required(VERSION 3.10)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  endif()
endif()

project(vitadeck)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(VITADECK_WERROR "Treat compiler warnings as errors (CI only; newer compilers add warnings)" OFF)
set(VITADECK_SANITIZE "" CACHE STRING "Host-only -fsanitize= list, e.g. address,undefined")

add_compile_options(-Wall -Wextra)
if(VITADECK_WERROR)
  add_compile_options(-Werror)
endif()

if(DEFINED ENV{VITASDK})
  message(STATUS "VITA is ON")
  include("${VITASDK}/share/vita.cmake" REQUIRED)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
  if(VITADECK_SANITIZE)
    message(FATAL_ERROR "VITADECK_SANITIZE is host-only")
  endif()
else()
  message(STATUS "VITA is OFF")
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Og -g")
  else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
  endif()
  if(VITADECK_SANITIZE)
    message(STATUS "Sanitizers: ${VITADECK_SANITIZE}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=${VITADECK_SANITIZE} -fno-sanitize-recover=all -fno-omit-frame-pointer -g")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=${VITADECK_SANITIZE}")
  endif()
endif()

set(VITA_APP_NAME "VitaDeck")
set(PSVITAIP "$ENV{PSVITAIP}")
set(VITA_TITLEID  "PGRI00001")
set(VITADECK_VERSION "" CACHE STRING "Release version X.Y.Z; the VPK gets XX.YY (patch dropped)")
set(VITA_VERSION "01.00")
if(VITADECK_VERSION)
  if(NOT VITADECK_VERSION MATCHES "^([0-9][0-9]?)\\.([0-9][0-9]?)\\.[0-9]+$")
    message(FATAL_ERROR "VITADECK_VERSION must be X.Y.Z with X and Y below 100, got '${VITADECK_VERSION}'")
  endif()
  set(vita_major "${CMAKE_MATCH_1}")
  set(vita_minor "${CMAKE_MATCH_2}")
  string(REGEX REPLACE "^([0-9])$" "0\\1" vita_major "${vita_major}")
  string(REGEX REPLACE "^([0-9])$" "0\\1" vita_minor "${vita_minor}")
  set(VITA_VERSION "${vita_major}.${vita_minor}")
endif()
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

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/vendor/quickjs/quickjs.c")
  find_package(Git QUIET)
  if(GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
    message(STATUS "QuickJS submodule is missing; initializing vendor/quickjs")
    execute_process(
      COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive vendor/quickjs
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      RESULT_VARIABLE QUICKJS_SUBMODULE_RESULT
    )
  endif()
  if(NOT EXISTS "${CMAKE_SOURCE_DIR}/vendor/quickjs/quickjs.c")
    message(FATAL_ERROR "Missing vendor/quickjs. Run `git submodule update --init --recursive vendor/quickjs` and re-run CMake.")
  endif()
endif()

# Set up include directories
set(VITADECK_INCLUDE_DIRS src)
# Third-party headers are not ours to fix; SYSTEM (-isystem) keeps their warnings
# out of our builds. System libcurl (e.g. macOS SDK) lives under usr/include, where
# -I would also make Clang diagnose libc headers.
set(VITADECK_SYSTEM_INCLUDE_DIRS vendor vendor/quickjs ${OPENSSL_INCLUDE_DIR} ${CURL_INCLUDE_DIRS})

if(DEFINED ENV{VITASDK})
  list(APPEND VITADECK_INCLUDE_DIRS
    $ENV{VITASDK}/arm-vita-eabi/include
    $ENV{VITASDK}/arm-vita-eabi/include/SDL2
  )
else()
  list(APPEND VITADECK_SYSTEM_INCLUDE_DIRS ${RAYLIB_INCLUDE_DIRS})
  list(APPEND VITADECK_LIBRARY_DIRS ${RAYLIB_LIBRARY_DIRS})
endif()

include_directories(${VITADECK_INCLUDE_DIRS})
include_directories(SYSTEM ${VITADECK_SYSTEM_INCLUDE_DIRS})
link_directories(${VITADECK_LIBRARY_DIRS})

# QuickJS
set(QUICKJS_SOURCES
    vendor/quickjs/quickjs.c
    vendor/quickjs/libregexp.c
    vendor/quickjs/libunicode.c
    vendor/quickjs/cutils.c
    vendor/quickjs/dtoa.c
)
file(STRINGS vendor/quickjs/VERSION QUICKJS_VERSION LIMIT_COUNT 1)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS vendor/quickjs/VERSION)
add_library(quickjs STATIC ${QUICKJS_SOURCES})
target_compile_options(quickjs PRIVATE -w)
if(VITADECK_SANITIZE)
  # QuickJS relies on signed-shift and similar UB upstream; ASan still covers it.
  target_compile_options(quickjs PRIVATE -fno-sanitize=undefined)
endif()
target_compile_definitions(quickjs PRIVATE
    CONFIG_VERSION="${QUICKJS_VERSION}"
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
    src/jslib/error.c
)

set(VITADECK_SOURCES
    src/main.c
    src/core/arena.c
    src/core/bootstrap.c
    src/core/js_runtime.c
    src/core/package_manifest.c
    src/core/package_library.c
    src/core/util.c
    ${JSLIB_SOURCES}
    src/ui/instance_tree.c
    src/ui/fonts.c
    src/ui/images.c
    src/ui/render.c
    src/ui/input.c
    src/ui/focus_nav.c
    src/ui/scroll.c
    src/core/event_queue.c
    src/shell/shell.c
    src/upload/archive.c
    src/upload/http_parse.c
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
    add_executable(timer_reload_harness tests/timer_reload_harness.c src/jslib/timeout.c src/jslib/error.c)
    target_link_libraries(timer_reload_harness quickjs m ${RAYLIB_LIBRARIES})
    add_test(NAME timer_reload_harness COMMAND timer_reload_harness)

    add_executable(package_manifest_harness tests/package_manifest_harness.c src/core/package_manifest.c src/core/util.c)
    add_test(NAME package_manifest_harness COMMAND package_manifest_harness)

    add_executable(package_library_harness
      tests/package_library_harness.c
      src/core/package_library.c
      src/core/package_manifest.c
      src/core/util.c
      ${PLATFORM_SOURCES}
    )
    add_test(NAME package_library_harness COMMAND package_library_harness)

    add_executable(upload_parsing_harness
      tests/upload_parsing_harness.c
      src/upload/archive.c
      src/upload/http_parse.c
      src/core/arena.c
      src/core/package_library.c
      src/core/package_manifest.c
      src/core/util.c
      ${PLATFORM_SOURCES}
    )
    target_link_libraries(upload_parsing_harness ZLIB::ZLIB)
    add_test(NAME upload_parsing_harness COMMAND upload_parsing_harness)

    add_executable(event_queue_harness tests/event_queue_harness.c src/core/event_queue.c ${PLATFORM_SOURCES})
    target_link_libraries(event_queue_harness pthread ${RAYLIB_LIBRARIES})
    add_test(NAME event_queue_harness COMMAND event_queue_harness)

    add_executable(snapshot_swap_harness
      tests/snapshot_swap_harness.c
      src/ui/instance_tree.c
      src/ui/scroll.c
      src/core/arena.c
      ${PLATFORM_SOURCES}
    )
    target_link_libraries(snapshot_swap_harness m pthread)
    add_test(NAME snapshot_swap_harness COMMAND snapshot_swap_harness)

    add_executable(hit_test_harness
      tests/hit_test_harness.c
      src/ui/instance_tree.c
      src/ui/focus_nav.c
      src/ui/scroll.c
      src/core/arena.c
      ${PLATFORM_SOURCES}
    )
    target_link_libraries(hit_test_harness m pthread)
    add_test(NAME hit_test_harness COMMAND hit_test_harness)

    add_executable(scroll_math_harness
      tests/scroll_math_harness.c
      src/ui/instance_tree.c
      src/ui/scroll.c
      src/core/arena.c
      ${PLATFORM_SOURCES}
    )
    target_link_libraries(scroll_math_harness m pthread)
    add_test(NAME scroll_math_harness COMMAND scroll_math_harness)

    # Links raylib only because input.c also holds the raylib polls; the harness drives input_apply_*.
    add_executable(input_harness
      tests/input_harness.c
      src/ui/input.c
      src/ui/instance_tree.c
      src/ui/focus_nav.c
      src/ui/scroll.c
      src/core/arena.c
      src/core/event_queue.c
      ${PLATFORM_SOURCES}
    )
    target_link_libraries(input_harness m pthread ${RAYLIB_LIBRARIES})
    add_test(NAME input_harness COMMAND input_harness)

    # Includes src/upload/http_server.c itself, to reach the static request handler.
    add_executable(upload_single_flight_harness
      tests/upload_single_flight_harness.c
      src/upload/archive.c
      src/upload/http_parse.c
      src/core/arena.c
      src/core/package_library.c
      src/core/package_manifest.c
      src/core/util.c
      ${PLATFORM_SOURCES}
    )
    target_link_libraries(upload_single_flight_harness ZLIB::ZLIB pthread)
    add_test(NAME upload_single_flight_harness COMMAND upload_single_flight_harness)

    # The native app without main.c, the shell and Runtime Upload, for harnesses that drive the real JS thread.
    set(HOST_RUNTIME_SOURCES
      src/core/arena.c
      src/core/bootstrap.c
      src/core/js_runtime.c
      src/core/package_manifest.c
      src/core/package_library.c
      src/core/util.c
      src/core/event_queue.c
      ${JSLIB_SOURCES}
      src/ui/instance_tree.c
      src/ui/fonts.c
      src/ui/images.c
      src/ui/render.c
      src/ui/input.c
      src/ui/focus_nav.c
      src/ui/scroll.c
      src/platform/thread_posix.c
    )
    add_executable(js_limits_harness tests/js_limits_harness.c ${HOST_RUNTIME_SOURCES})
    target_link_libraries(js_limits_harness ${VITADECK_LIBRARIES})
    add_test(NAME js_limits_harness COMMAND js_limits_harness)

    # Same checks on the Vita JS thread stack, to prove the QuickJS stack headroom fits it.
    add_executable(js_limits_vita_stack_harness tests/js_limits_harness.c ${HOST_RUNTIME_SOURCES})
    target_compile_definitions(js_limits_vita_stack_harness PRIVATE VD_THREAD_STACK_SIZE=VD_VITA_THREAD_STACK_SIZE)
    target_link_libraries(js_limits_vita_stack_harness ${VITADECK_LIBRARIES})
    add_test(NAME js_limits_vita_stack_harness COMMAND js_limits_vita_stack_harness)

    add_executable(smoke_harness tests/smoke_harness.c ${HOST_RUNTIME_SOURCES})
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
    if(VITADECK_SANITIZE)
      set_tests_properties(smoke_harness PROPERTIES
        ENVIRONMENT "LSAN_OPTIONS=suppressions=${CMAKE_SOURCE_DIR}/tests/lsan.supp"
      )
    endif()
  endif()
endif()

add_custom_target(js_build
  COMMAND pnpm build
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
  COMMAND pnpm build
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ${PROJECT_NAME}
  COMMENT "Build JS bundle (pnpm build) then native ${PROJECT_NAME}"
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

  if(PSVITAIP)
    add_custom_target(require_psvitaip)
  else()
    add_custom_target(require_psvitaip
      COMMAND ${CMAKE_COMMAND} -E echo "PSVITAIP is unset: export PSVITAIP=<Vita IP> and re-run cmake"
      COMMAND ${CMAKE_COMMAND} -E false
      VERBATIM
    )
  endif()

  # Create an explicit `upload_vpk` target to upload the VPK file to Vita's download folder
  add_custom_target(upload_vpk
    DEPENDS ${PROJECT_NAME}.vpk-vpk
    COMMAND echo "Sending ${PROJECT_NAME}.vpk to vita download folder"
    COMMAND curl -T ${PROJECT_NAME}.vpk ftp://${PSVITAIP}:1337/ux0:/download/
    COMMAND echo "VPK uploaded to ux0:/download/"
    COMMENT "Upload VPK file to Vita download folder"
    VERBATIM
  )
  add_dependencies(upload_vpk require_psvitaip)

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
  add_dependencies(upload_vita require_psvitaip)

  # Add a convenience target `run` that depends on upload_vita so `make run` will
  # build, upload, and launch the app on the Vita.
  add_custom_target(run
    DEPENDS upload_vita
    COMMENT "Build, upload, and launch on Vita"
    VERBATIM
  )
else()
  add_dependencies(${PROJECT_NAME} js_copy)

  add_custom_target(stage_examples
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/stage-example-apps.sh ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}
    COMMENT "Staging example Deck Apps to ${CMAKE_BINARY_DIR}/vitadeck-data/installed-deck-apps"
    VERBATIM
  )

  add_custom_target(run
    DEPENDS ${PROJECT_NAME} stage_examples
    VERBATIM
    COMMAND ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
  )
endif()
