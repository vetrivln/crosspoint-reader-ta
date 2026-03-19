#pragma once

#include <cstdint>
#include <vector>

namespace TamilShaper {

/**
 * Represents a positioned glyph within a Tamil cluster.
 *
 * Tamil clusters consist of a base consonant (or independent vowel) with
 * optional attached vowel signs and combining marks. Glyphs in the same
 * cluster share the cursor position; only glyphs with zeroAdvance = false
 * move the cursor forward.
 */
struct PositionedGlyph {
  uint32_t codepoint;  // Unicode codepoint to render
  int8_t xOffset;      // X offset from cluster origin (signed font units)
  int8_t yOffset;      // Y offset from baseline (negative = up, positive = down)
  bool zeroAdvance;    // If true, glyph does not advance the cursor
};

/**
 * A Tamil grapheme cluster — the minimal correctly renderable unit.
 *
 * A typical Tamil cluster consists of:
 *   [left-vowel]  base-consonant  [above/below/right vowel]  [virama]
 *
 * Split vowels add one glyph before AND one glyph after the consonant.
 *
 * Examples:
 *   க   → single consonant, no marks
 *   கா  → க + ா (right vowel, displayed after)
 *   கி  → க + ி (above vowel)
 *   கு  → க + ு (below vowel)
 *   கெ  → ெ + க  (left vowel reordered before base)
 *   கொ  → ெ + க + ா (split vowel: left + base + right)
 *   க்  → க + ் (virama, displayed above/after)
 */
struct TamilCluster {
  std::vector<PositionedGlyph> glyphs;
  int totalAdvance;  // Total width of cluster in font advanceX units

  TamilCluster() : totalAdvance(0) {}
};

/**
 * Y-offset constants for Tamil mark positioning.
 *
 * Tamil fonts typically place combining marks at fixed positions relative
 * to the base consonant. The values below are suitable for NotoSansTamil
 * at the sizes used by the e-ink reader (8–18 pt at 150 dpi).
 *
 * Negative values move UP; positive values move DOWN.
 * X offsets are 0 for most Tamil marks (they are designed to center on
 * the base glyph in the font metrics).
 */
namespace TamilOffset {
  // Vowel signs that float above the consonant body
  constexpr int8_t ABOVE_VOWEL  = -2;  // ி ீ

  // Vowel signs that hang below the consonant
  constexpr int8_t BELOW_VOWEL  =  3;  // ு ூ

  // Virama (pulli) sits above/slightly right of the consonant
  constexpr int8_t VIRAMA       = -2;

  // Anusvara sits above the consonant, higher than above-vowels
  constexpr int8_t ANUSVARA     = -4;
}  // namespace TamilOffset

}  // namespace TamilShaper
