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

# The CJK Runtime Fallback Font from AOSP platform/frameworks/base at tag android-7.1.2_r39, through its GitHub mirror.
vitadeck_fetch_asset(DroidSansFallbackFull
  URL "https://raw.githubusercontent.com/aosp-mirror/platform_frameworks_base/8fe116528f1caf04f1a1edd07ec054cf463cc985/data/fonts/DroidSansFallbackFull.ttf"
  SHA256 acb6440a713d880a13a21b468ba7cd43f5a2b2934972e51be791c880730777b8
  DIR "${CMAKE_SOURCE_DIR}/assets/fonts"
  FALLBACK "Characters only the Runtime Fallback Font has, like CJK, draw as the missing-glyph box."
)
if(DroidSansFallbackFull_FOUND)
  list(APPEND VITADECK_RUNTIME_FONTS DroidSansFallbackFull.ttf)
endif()

# The Korean Runtime Fallback Font, Noto Sans KR Regular, from the notofonts/noto-cjk Sans2.004 tag's commit. The
# Korean subset build covers every Hangul syllable; cutting it to Hangul only would need a font tool at build time.
vitadeck_fetch_asset(NotoSansKR
  URL "https://raw.githubusercontent.com/notofonts/noto-cjk/523d033d6cb47f4a80c58a35753646f5c3608a78/Sans/SubsetOTF/KR/NotoSansKR-Regular.otf"
  SHA256 69975a0ac8472717870aefeab0a4d52739308d90856b9955313b2ad5e0148d68
  DIR "${CMAKE_SOURCE_DIR}/assets/fonts"
  FALLBACK "Korean Hangul draws as the missing-glyph box."
)
if(NotoSansKR_FOUND)
  list(APPEND VITADECK_RUNTIME_FONTS NotoSansKR-Regular.otf)
endif()
