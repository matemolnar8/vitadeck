# Colour emoji images: Twemoji's 72px PNGs, downloaded once at configure time into the gitignored assets/emoji.
set(VITADECK_TWEMOJI_VERSION 17.0.3)
vitadeck_fetch_asset(Twemoji
  URL "https://github.com/jdecked/twemoji/archive/refs/tags/v${VITADECK_TWEMOJI_VERSION}.zip"
  SHA256 d8a269885d95c78123cc0981c8c76a4c55466cdcb6c7c939434323533e6cd754
  DIR "${CMAKE_SOURCE_DIR}/assets/emoji"
  FALLBACK "The build has no emoji images, so emoji draw a fallback glyph."
  EXTRACT "twemoji-${VITADECK_TWEMOJI_VERSION}/assets/72x72/*.png"
  # The emoji loader names images with U+FE0F left out.
  STRIP "-fe0f"
)
