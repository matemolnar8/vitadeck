# Runtime fonts, downloaded once at configure time into the gitignored assets/fonts; VITADECK_RUNTIME_FONTS lists the
# ones present, for the VPK.
set(VITADECK_RUNTIME_FONTS "")

# The Runtime Fallback Font from AOSP platform/frameworks/base at tag android-7.1.2_r39, through its GitHub mirror.
vitadeck_fetch_asset(DroidSansFallbackFull
  URL "https://raw.githubusercontent.com/aosp-mirror/platform_frameworks_base/8fe116528f1caf04f1a1edd07ec054cf463cc985/data/fonts/DroidSansFallbackFull.ttf"
  SHA256 acb6440a713d880a13a21b468ba7cd43f5a2b2934972e51be791c880730777b8
  DIR "${CMAKE_SOURCE_DIR}/assets/fonts"
  FALLBACK "Characters only the Runtime Fallback Font has, like CJK, draw as the missing-glyph box."
)
if(DroidSansFallbackFull_FOUND)
  list(APPEND VITADECK_RUNTIME_FONTS DroidSansFallbackFull.ttf)
endif()
