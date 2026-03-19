#include "GfxRenderer.h"

#include <Logging.h>
#include <TamilShaper.h>
#include "FontCacheManager.h"

namespace {

/**
 * Render a single PositionedGlyph from a TamilCluster onto the screen.
 */
static void renderTamilGlyph(const GfxRenderer& renderer,
                              GfxRenderer::RenderMode renderMode,
                              const EpdFontFamily& font,
                              const TamilShaper::PositionedGlyph& g,
                              int cursorX, int baseY,
                              bool black,
                              EpdFontFamily::Style style) {
  const EpdGlyph* epglyph = font.getGlyph(g.codepoint, style);
  if (!epglyph) {
    LOG_ERR("GFX", "Tamil: no glyph for U+%04X", g.codepoint);
    return;
  }

  const EpdFontData* fontData = font.getData(style);
  const bool is2Bit = fontData->is2Bit;
  const uint8_t width  = epglyph->width;
  const uint8_t height = epglyph->height;
  const int left = epglyph->left;
  const int top  = epglyph->top;

  const uint8_t* bitmap = renderer.getGlyphBitmap(fontData, epglyph);
  if (!bitmap) return;

  const int drawX = cursorX + g.xOffset;
  const int drawY = baseY  + g.yOffset;

  const int outerBase = drawY - top;
  const int innerBase = drawX + left;

  if (is2Bit) {
    int pixelPosition = 0;
    for (int gy = 0; gy < height; gy++) {
      const int screenY = outerBase + gy;
      for (int gx = 0; gx < width; gx++, pixelPosition++) {
        const int screenX = innerBase + gx;
        const uint8_t byte    = bitmap[pixelPosition >> 2];
        const uint8_t bit_idx = (3 - (pixelPosition & 3)) * 2;
        const uint8_t bmpVal  = 3 - ((byte >> bit_idx) & 0x3);
        if (renderMode == GfxRenderer::BW && bmpVal < 3) {
          renderer.drawPixel(screenX, screenY, black);
        } else if (renderMode == GfxRenderer::GRAYSCALE_MSB && (bmpVal == 1 || bmpVal == 2)) {
          renderer.drawPixel(screenX, screenY, false);
        } else if (renderMode == GfxRenderer::GRAYSCALE_LSB && bmpVal == 1) {
          renderer.drawPixel(screenX, screenY, false);
        }
      }
    }
  } else {
    int pixelPosition = 0;
    for (int gy = 0; gy < height; gy++) {
      const int screenY = outerBase + gy;
      for (int gx = 0; gx < width; gx++, pixelPosition++) {
        const int screenX = innerBase + gx;
        const uint8_t byte    = bitmap[pixelPosition >> 3];
        const uint8_t bit_idx = 7 - (pixelPosition & 7);
        if ((byte >> bit_idx) & 1) {
          renderer.drawPixel(screenX, screenY, black);
        }
      }
    }
  }
}

}  // namespace

void GfxRenderer::drawTextTamil(const int fontId, const int x, const int y,
                                 const char* text, const bool black,
                                 EpdFontFamily::Style style) const {
  if (text == nullptr || *text == '\0') return;

  if (fontCacheManager_ && fontCacheManager_->isScanning()) {
    fontCacheManager_->recordText(text, fontId, style);
    return;
  }

  const auto fontIt = fontMap.find(fontId);
  if (fontIt == fontMap.end()) {
    LOG_ERR("GFX", "Tamil: font %d not found", fontId);
    return;
  }
  const auto& font = fontIt->second;

  const int baseY = y + getFontAscenderSize(fontId);

  auto clusters = TamilShaper::TamilClusterBuilder::buildClusters(text);

  int32_t xPosFP = fp4::fromPixel(x);

  for (const auto& cluster : clusters) {
    int32_t baseRightFP = xPosFP;

    for (const auto& g : cluster.glyphs) {
      if (!TamilShaper::isTamilCodepoint(g.codepoint) && !g.zeroAdvance) {
        const uint32_t cp = g.codepoint;
        const int renderX = fp4::toPixel(xPosFP);
        renderCodepoint(fontId, cp, renderX, baseY, black, style);
        const auto latinIt = fontMap.find(fontId);
        if (latinIt != fontMap.end()) {
          const EpdGlyph* fg = latinIt->second.getGlyph(cp, style);
          if (fg) {
            baseRightFP = xPosFP + fg->advanceX;
            xPosFP     += fg->advanceX;
          }
        }
        continue;
      }

      if (g.zeroAdvance) {
        const int renderX = fp4::toPixel(baseRightFP);
        renderTamilGlyph(*this, renderMode, font, g, renderX, baseY, black, style);
      } else {
        const int renderX = fp4::toPixel(xPosFP);
        renderTamilGlyph(*this, renderMode, font, g, renderX, baseY, black, style);
        const EpdGlyph* epglyph = font.getGlyph(g.codepoint, style);
        if (epglyph) {
          baseRightFP = xPosFP + epglyph->advanceX;
          xPosFP     += epglyph->advanceX;
        }
      }
    }
  }
}

int GfxRenderer::getTextAdvanceXTamil(const int fontId, const char* text,
                                       EpdFontFamily::Style style) const {
  if (text == nullptr || *text == '\0') return 0;

  const auto fontIt = fontMap.find(fontId);
  if (fontIt == fontMap.end()) {
    LOG_ERR("GFX", "Tamil: font %d not found", fontId);
    return 0;
  }
  const auto& font = fontIt->second;

  auto clusters = TamilShaper::TamilClusterBuilder::buildClusters(text);

  int32_t widthFP = 0;

  for (const auto& cluster : clusters) {
    for (const auto& g : cluster.glyphs) {
      if (!g.zeroAdvance) {
        // Non-Tamil substitution glyph — measure from Latin font
        if (!TamilShaper::isTamilCodepoint(g.codepoint)) {
          const auto latinIt = fontMap.find(fontId);
          if (latinIt != fontMap.end()) {
            const EpdGlyph* fg = latinIt->second.getGlyph(g.codepoint, style);
            if (fg) widthFP += fg->advanceX;
          }
          continue;
        }
        const EpdGlyph* epglyph = font.getGlyph(g.codepoint, style);
        if (epglyph) widthFP += epglyph->advanceX;
      }
    }
  }

  return fp4::toPixel(widthFP);
}
