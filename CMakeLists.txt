cmake_minimum_required(VERSION 3.14)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  endif()
endif()

project(vitadeck)

include(cmake/fetch_asset.cmake)
include(cmake/twemoji.cmake)
include(cmake/fonts.cmake)

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

if(DEFINED ENV{VITASDK})
  set(PLATFORM_SOURCES src/platform/thread_vita.c src/platform/text_input_vita.c)
else()
  set(PLATFORM_SOURCES src/platform/thread_posix.c src/platform/text_input_host.c)
endif()

set(JSLIB_SOURCES
    src/jslib/jslib.c
    src/jslib/instance.c
    src/jslib/colors.c
    src/jslib/timeout.c
    src/jslib/log.c
    src/jslib/fetch.c
    src/jslib/error.c
    src/jslib/storage.c
)

# Everything but main.c, so the app and the test harnesses compile each source once.
set(VITADECK_CORE_SOURCES
    src/core/arena.c
    src/core/bench.c
    src/core/bootstrap.c
    src/core/deck_app_storage.c
    src/core/js_runtime.c
    src/core/package_manifest.c
    src/core/package_library.c
    src/core/util.c
    ${JSLIB_SOURCES}
    src/ui/instance_tree.c
    src/ui/layout.c
    src/ui/fonts.c
    src/ui/glyph_cache.c
    src/ui/text.c
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

set(VITADECK_LIBRARIES m)

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

add_library(vitadeck_core STATIC ${VITADECK_CORE_SOURCES})
target_link_libraries(vitadeck_core PUBLIC ${VITADECK_LIBRARIES})
# QuickJS links next to vitadeck_core rather than into it: a link dependency would make Make compile them one after
# the other.
set(VITADECK_CORE_LIBRARIES vitadeck_core quickjs)

add_executable(${PROJECT_NAME} src/main.c)
target_link_libraries(${PROJECT_NAME} ${VITADECK_CORE_LIBRARIES})

if(NOT DEFINED ENV{VITASDK})
  # pnpm build makes the SDK, the example Deck Apps, and the runtime together; runtime.js stands for all of them.
  # It runs only when these inputs change. The Vita build packs whatever an earlier pnpm build left in js/dist.
  file(GLOB js_inputs CONFIGURE_DEPENDS
    js/*.json js/*.yaml js/packages/*/*.json js/packages/*/*.js js/packages/*/*.mjs js/packages/sdk/cli/*.ts
    js/packages/sdk/cli/templates/*/*.json js/packages/sdk/cli/templates/*/.gitignore js/examples/*/*.json
  )
  file(GLOB_RECURSE js_sources CONFIGURE_DEPENDS
    js/packages/*/src/* js/packages/sdk/cli/templates/*/src/* js/examples/*/src/* js/examples/*/assets/*
  )
  set(js_runtime ${CMAKE_SOURCE_DIR}/js/dist/runtime/runtime.js)
  add_custom_command(OUTPUT ${js_runtime}
    COMMAND pnpm build
    COMMAND ${CMAKE_COMMAND} -E touch ${js_runtime}
    DEPENDS ${js_inputs} ${js_sources}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/js
    COMMENT "Building JavaScript with pnpm"
    VERBATIM
  )
  add_custom_target(js ALL DEPENDS ${js_runtime})
  add_dependencies(${PROJECT_NAME} js)

  # The host reads js/runtime.js and assets/ relative to its working directory; links instead of copies keep them
  # current. Build trees from before the links held copies, which CREATE_LINK cannot replace. In the source tree
  # those names are the real js/ and assets/.
  if(CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "In-source builds are not supported; configure a build directory, e.g. cmake -S . -B out")
  endif()
  function(link_into_build name source)
    set(link ${CMAKE_BINARY_DIR}/${name})
    if(IS_DIRECTORY ${link} AND NOT IS_SYMLINK ${link})
      file(REMOVE_RECURSE ${link})
    endif()
    file(CREATE_LINK ${CMAKE_SOURCE_DIR}/${source} ${link} SYMBOLIC)
  endfunction()
  link_into_build(js js/dist/runtime)
  link_into_build(assets assets)

  include(CTest)
  if(BUILD_TESTING)
    foreach(harness
      timer_reload package_manifest package_library upload_parsing storage event_queue snapshot_swap hit_test
      scroll_math input upload_single_flight shell_input js_limits input_budget
    )
      add_executable(${harness}_harness tests/${harness}_harness.c)
      target_link_libraries(${harness}_harness ${VITADECK_CORE_LIBRARIES})
      add_test(NAME ${harness}_harness COMMAND ${harness}_harness)
    endforeach()

    add_executable(layout_harness tests/layout_harness.c)
    target_link_libraries(layout_harness ${VITADECK_CORE_LIBRARIES})
    # The Runtime Default Font loads from assets/fonts, relative to the working directory.
    add_test(NAME layout_harness COMMAND layout_harness WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

    add_executable(glyph_cache_harness tests/glyph_cache_harness.c)
    target_compile_definitions(glyph_cache_harness PRIVATE VD_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
    if(Twemoji_FOUND)
      target_compile_definitions(glyph_cache_harness PRIVATE VD_TWEMOJI_DOWNLOADED)
    endif()
    target_link_libraries(glyph_cache_harness ${VITADECK_CORE_LIBRARIES})
    add_test(NAME glyph_cache_harness COMMAND glyph_cache_harness)

    # Same checks on the Vita JS thread stack, to prove the QuickJS stack headroom fits it. Its own copies of the two
    # sources that read VD_THREAD_STACK_SIZE take the place of the ones in vitadeck_core.
    add_executable(js_limits_vita_stack_harness tests/js_limits_harness.c src/core/js_runtime.c src/platform/thread_posix.c)
    target_compile_definitions(js_limits_vita_stack_harness PRIVATE VD_THREAD_STACK_SIZE=VD_VITA_THREAD_STACK_SIZE)
    target_link_libraries(js_limits_vita_stack_harness ${VITADECK_CORE_LIBRARIES})
    add_test(NAME js_limits_vita_stack_harness COMMAND js_limits_vita_stack_harness)

    add_executable(smoke_harness tests/smoke_harness.c)
    target_link_libraries(smoke_harness ${VITADECK_CORE_LIBRARIES})
    add_dependencies(smoke_harness js)
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

    # Presses every Pad Button in each example Deck App and switches to the next; the harness's own watchdog
    # reports a hang, TIMEOUT is the backstop.
    add_executable(deck_app_input_harness tests/deck_app_input_harness.c)
    target_link_libraries(deck_app_input_harness ${VITADECK_CORE_LIBRARIES})
    add_dependencies(deck_app_input_harness js)
    add_test(
      NAME deck_app_input_harness
      COMMAND ${CMAKE_SOURCE_DIR}/tests/run_deck_app_input_test.sh ${CMAKE_BINARY_DIR}
    )
    set_tests_properties(deck_app_input_harness PROPERTIES
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      LABELS "smoke"
      TIMEOUT 300
    )
    if(VITADECK_SANITIZE)
      set_tests_properties(deck_app_input_harness PROPERTIES
        ENVIRONMENT "LSAN_OPTIONS=suppressions=${CMAKE_SOURCE_DIR}/tests/lsan.supp"
      )
    endif()
  endif()
endif()

add_custom_target(cloc
  COMMENT "Count lines of code in ${CMAKE_SOURCE_DIR}"
  COMMAND cloc ${CMAKE_SOURCE_DIR} --match-d=src "--not-match-d=(out|node_modules)"
  VERBATIM
)

if(DEFINED ENV{VITASDK})
  set(VITA_ELF_CREATE_FLAGS "${VITA_ELF_CREATE_FLAGS}")

  vita_create_self(eboot.bin ${PROJECT_NAME} UNSAFE NOSTRIP)

  set(vpk_fonts "")
  set(upload_fonts "")
  foreach(font IN LISTS VITADECK_RUNTIME_FONTS)
    list(APPEND vpk_fonts FILE assets/fonts/${font} assets/fonts/${font})
    list(APPEND upload_fonts
      COMMAND echo "Sending ${VITA_TITLEID}/assets/fonts/${font} to vita"
      COMMAND curl --ftp-create-dirs -T ${CMAKE_SOURCE_DIR}/assets/fonts/${font} ftp://${PSVITAIP}:1337/ux0:/app/${VITA_TITLEID}/assets/fonts/${font}
    )
  endforeach()

  vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} eboot.bin
      VERSION ${VITA_VERSION}
      NAME ${VITA_APP_NAME}
      FILE module module
      FILE js/dist/runtime/runtime.js js/runtime.js
      ${vpk_fonts}
      FILE assets/fonts/DejaVuSans-LICENSE.txt assets/fonts/DejaVuSans-LICENSE.txt
      FILE assets/fonts/DroidSansFallbackFull-LICENSE.txt assets/fonts/DroidSansFallbackFull-LICENSE.txt
      FILE assets/fonts/NotoSansKR-LICENSE.txt assets/fonts/NotoSansKR-LICENSE.txt
      FILE assets/emoji assets/emoji
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

  # Emoji images go up in one curl run over one FTP connection, and only again when they change; the stamp is
  # per Vita IP. Delete it to force a re-upload, e.g. after the app was removed from the Vita.
  file(GLOB upload_emoji_files ${CMAKE_SOURCE_DIR}/assets/emoji/*.png)
  set(upload_emoji_stamp "")
  if(upload_emoji_files)
    set(upload_emoji_stamp ${CMAKE_BINARY_DIR}/upload_emoji-${PSVITAIP}.stamp)
    list(LENGTH upload_emoji_files upload_emoji_count)
    set(upload_emoji_config "ftp-create-dirs\nsilent\nshow-error\nwrite-out = \"\\r  %{urlnum}/${upload_emoji_count}\"\n")
    foreach(file IN LISTS upload_emoji_files)
      get_filename_component(name ${file} NAME)
      string(APPEND upload_emoji_config "upload-file = \"${file}\"\nurl = \"ftp://${PSVITAIP}:1337/ux0:/app/${VITA_TITLEID}/assets/emoji/${name}\"\n")
    endforeach()
    file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/upload_emoji.curl CONTENT "${upload_emoji_config}")
    add_custom_command(OUTPUT ${upload_emoji_stamp}
      COMMAND echo "Sending ${upload_emoji_count} emoji images to ${VITA_TITLEID}/assets/emoji (a few minutes)"
      COMMAND curl -K ${CMAKE_BINARY_DIR}/upload_emoji.curl
      COMMAND echo ""
      COMMAND ${CMAKE_COMMAND} -E touch ${upload_emoji_stamp}
      DEPENDS ${CMAKE_BINARY_DIR}/upload_emoji.curl ${upload_emoji_files}
      VERBATIM
    )
  endif()

  # Create an explicit `upload_vita` target so uploads only run when requested.
  add_custom_target(upload_vita
    DEPENDS ${PROJECT_NAME}.vpk-vpk ${upload_emoji_stamp}
    # Destroy running apps
    COMMAND echo "Destroy running apps"
    COMMAND echo destroy | nc ${PSVITAIP} 1338
    # Upload eboot.bin
    COMMAND echo "Sending ${VITA_TITLEID}/eboot.bin to vita"
    COMMAND curl -T eboot.bin ftp://${PSVITAIP}:1337/ux0:/app/${VITA_TITLEID}/
    # Upload runtime and active Deck App package
    COMMAND echo "Sending ${VITA_TITLEID}/js/runtime.js to vita"
    COMMAND curl -T ${CMAKE_SOURCE_DIR}/js/dist/runtime/runtime.js ftp://${PSVITAIP}:1337/ux0:/app/${VITA_TITLEID}/js/runtime.js
    ${upload_fonts}
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
  add_custom_target(stage_examples
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/stage-example-apps.sh ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}/vitadeck-data
    COMMENT "Staging example Deck Apps to ${CMAKE_BINARY_DIR}/vitadeck-data/installed-deck-apps"
    VERBATIM
  )
  add_dependencies(stage_examples js)

  add_custom_target(run
    DEPENDS ${PROJECT_NAME} stage_examples
    VERBATIM
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/check-macos-display.sh
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/gui-run.sh ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
  )
endif()
