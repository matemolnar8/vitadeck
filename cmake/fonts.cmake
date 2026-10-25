# Runtime fonts, downloaded once at configure time into the gitignored assets/fonts; VITADECK_RUNTIME_FONTS lists the
# ones present, for the VPK.
set(VITADECK_RUNTIME_FONTS "")

# The Runtime Default Font from the DejaVu 2.37 release.
vitadeck_fetch_asset(DejaVuSans
  URL "https://github.com/dejavu-fonts/dejavu-fonts/releases/download/version_2_37/dejavu-sans-ttf-2.37.zip"
  SHA256 5c6e497a2f36552cb5ffb112c413a6af39c0f3c47653662b90b4fa6499822fd7
  DIR "${CMAKE_SOURCE_DIR}/assets/fonts"
  FALLBACK "Text in the Runtime Default Font draws with raylib's built-in font."
  EXTRACT "dejavu-sans-ttf-2.37/ttf/DejaVuSans.ttf"
)
if(DejaVuSans_FOUND)
  list(APPEND VITADECK_RUNTIME_FONTS DejaVuSans.ttf)
endif()

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
