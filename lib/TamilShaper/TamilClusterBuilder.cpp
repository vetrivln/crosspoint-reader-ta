#include "TamilClusterBuilder.h"

#include <Utf8.h>

// Debug logging for Tamil rendering investigation
// Set to 1 to enable verbose cluster building logging
#define TAMIL_CLUSTER_DEBUG_LOGGING 0

#if TAMIL_CLUSTER_DEBUG_LOGGING
#include <Arduino.h>
#endif

namespace TamilShaper {

std::vector<TamilCluster> TamilClusterBuilder::buildClusters(const char* text) {
  std::vector<TamilCluster> clusters;

  if (text == nullptr || *text == '\0') {
    return clusters;
  }

#if TAMIL_CLUSTER_DEBUG_LOGGING
  Serial.printf("[TAMIL] buildClusters input bytes: ");
  const uint8_t* debugPtr = reinterpret_cast<const uint8_t*>(text);
  for (int i = 0; i < 32 && debugPtr[i] != '\0'; i++) {
    Serial.printf("%02X ", debugPtr[i]);
  }
  Serial.printf("\n");
#endif

  const uint8_t* ptr = reinterpret_cast<const uint8_t*>(text);

  while (*ptr != '\0') {
    TamilCluster cluster = buildNextCluster(&ptr);
    if (!cluster.glyphs.empty()) {
      clusters.push_back(std::move(cluster));
    }
  }

#if TAMIL_CLUSTER_DEBUG_LOGGING
  Serial.printf("[TAMIL] Built %zu clusters\n", clusters.size());
#endif

  return clusters;
}

TamilCluster TamilClusterBuilder::buildNextCluster(const uint8_t** text) {
  TamilCluster cluster;

  if (*text == nullptr || **text == '\0') {
    return cluster;
  }

  // ── Peek at first codepoint ──────────────────────────────────────────────
  const uint8_t* peekPtr = *text;
  uint32_t firstCp = utf8NextCodepoint(&peekPtr);

#if TAMIL_CLUSTER_DEBUG_LOGGING
  Serial.printf("[TAMIL] buildNextCluster first cp: U+%04X\n", firstCp);
#endif

  // ── Non-Tamil: single-glyph passthrough ─────────────────────────────────
  if (!isTamilCodepoint(firstCp)) {
    utf8NextCodepoint(text);
    PositionedGlyph g;
    g.codepoint   = firstCp;
    g.xOffset     = 0;
    g.yOffset     = 0;
    g.zeroAdvance = false;
    cluster.glyphs.push_back(g);
    return cluster;
  }

  // ── Collect raw codepoints that belong to this cluster ───────────────────
  // Slots for the various parts of a Tamil syllable:
  uint32_t baseConsonant  = 0;  // or independent vowel acting as base
  uint32_t leftVowel      = 0;  // vowel displayed LEFT of base (ெ ே ை)
  uint32_t leftSplit      = 0;  // left half of a split vowel (ொ→ெ, ோ→ே, ௌ→ெ)
  uint32_t rightSplit     = 0;  // right half of a split vowel (ொ→ா, etc.)
  uint32_t aboveVowel     = 0;  // vowel above base (ி ீ)
  uint32_t belowVowel     = 0;  // vowel below base (ு ூ)
  uint32_t rightVowel     = 0;  // vowel to right of base (ா)
  uint32_t virama         = 0;  // pulli / virama (்)
  uint32_t anusvara       = 0;  // ஂ

  TamilCharType firstType = getTamilCharType(firstCp);

  // Independent vowels and standalone symbols are single-glyph clusters
  if (firstType == TamilCharType::INDEPENDENT_VOWEL ||
      firstType == TamilCharType::VISARGA ||
      firstType == TamilCharType::TAMIL_DIGIT ||
      firstType == TamilCharType::TAMIL_SYMBOL) {
    utf8NextCodepoint(text);
    PositionedGlyph g;
    g.codepoint   = firstCp;
    g.xOffset     = 0;
    g.yOffset     = 0;
    g.zeroAdvance = false;
    cluster.glyphs.push_back(g);
    return cluster;
  }

  // Must be a consonant to start a proper syllable cluster
  if (firstType != TamilCharType::CONSONANT) {
    // Orphaned combining mark — emit as-is and move on
    utf8NextCodepoint(text);
    PositionedGlyph g;
    g.codepoint   = firstCp;
    g.xOffset     = 0;
    g.yOffset     = 0;
    g.zeroAdvance = false;
    cluster.glyphs.push_back(g);
    return cluster;
  }

  // Consume the base consonant
  baseConsonant = firstCp;
  utf8NextCodepoint(text);

  // ── Consume combining marks that attach to this consonant ────────────────
  while (**text != '\0') {
    peekPtr = *text;
    uint32_t cp = utf8NextCodepoint(&peekPtr);

    if (!isTamilCodepoint(cp)) break;

    TamilCharType type = getTamilCharType(cp);

    switch (type) {
      case TamilCharType::ABOVE_VOWEL:
        if (aboveVowel != 0) goto done_parsing;
        aboveVowel = cp;
        utf8NextCodepoint(text);
        break;

      case TamilCharType::BELOW_VOWEL:
        if (belowVowel != 0) goto done_parsing;
        belowVowel = cp;
        utf8NextCodepoint(text);
        break;

      case TamilCharType::RIGHT_VOWEL:
        if (rightVowel != 0) goto done_parsing;
        rightVowel = cp;
        utf8NextCodepoint(text);
        break;

      case TamilCharType::LEFT_VOWEL:
        // In NFC Tamil, the left vowel follows the consonant in storage.
        // Treat as single left-vowel attachment; ends the cluster.
        if (leftVowel != 0) goto done_parsing;
        leftVowel = cp;
        utf8NextCodepoint(text);
        goto done_parsing;  // left vowels always close the syllable

      case TamilCharType::SPLIT_VOWEL_LEFT: {
        // Decompose into left + right parts
        uint32_t lp = 0, rp = 0;
        if (!getSplitVowelParts(cp, lp, rp)) goto done_parsing;
        if (leftSplit != 0) goto done_parsing;
        leftSplit  = lp;
        rightSplit = rp;
        utf8NextCodepoint(text);
        goto done_parsing;  // split vowels always close the syllable
      }

      case TamilCharType::VIRAMA:
        if (virama != 0) goto done_parsing;
        virama = cp;
        utf8NextCodepoint(text);
        // Do NOT consume the next consonant here — it starts a new cluster.
        // The pulli mark renders on THIS consonant and is purely suppressive.
        goto done_parsing;

      case TamilCharType::ANUSVARA:
        if (anusvara != 0) goto done_parsing;
        anusvara = cp;
        utf8NextCodepoint(text);
        goto done_parsing;

      default:
        // Consonant or anything else → new cluster
        goto done_parsing;
    }
  }

done_parsing:
  // ── Assemble positioned glyphs in visual order ───────────────────────────
  //
  // Visual order for a Tamil syllable:
  //   [left-vowel or left-split-half]
  //   [base consonant]
  //   [above vowel]        (zeroAdvance, yOffset up)
  //   [below vowel]        (zeroAdvance, yOffset down)
  //   [right vowel]        (advances cursor)
  //   [right-split-half]   (advances cursor)
  //   [virama]             (zeroAdvance, above)
  //   [anusvara]           (zeroAdvance, above)

  // 1. Left-side vowel (displayed before base)
  if (leftVowel != 0) {
    PositionedGlyph g;
    g.codepoint   = leftVowel;
    g.xOffset     = 0;
    g.yOffset     = 0;
    g.zeroAdvance = false;  // left vowel has its own advance width
    cluster.glyphs.push_back(g);
  } else if (leftSplit != 0) {
    PositionedGlyph g;
    g.codepoint   = leftSplit;
    g.xOffset     = 0;
    g.yOffset     = 0;
    g.zeroAdvance = false;
    cluster.glyphs.push_back(g);
  }

  // 2. Base consonant
  {
    PositionedGlyph g;
    g.codepoint   = baseConsonant;
    g.xOffset     = 0;
    g.yOffset     = 0;
    g.zeroAdvance = false;
    cluster.glyphs.push_back(g);
  }

  // 3. Above vowel sign (ி ீ)
  if (aboveVowel != 0) {
    PositionedGlyph g;
    g.codepoint   = aboveVowel;
    g.xOffset     = 0;
    g.yOffset     = TamilOffset::ABOVE_VOWEL;
    g.zeroAdvance = false;
    cluster.glyphs.push_back(g);
  }

  // 4. Below vowel sign (ு ூ)
  if (belowVowel != 0) {
    PositionedGlyph g;
    g.codepoint   = belowVowel;
    g.xOffset     = 0;
    g.yOffset     = TamilOffset::BELOW_VOWEL;
    g.zeroAdvance = false;
    cluster.glyphs.push_back(g);
  }

  // 5. Right vowel sign (ா)
  if (rightVowel != 0) {
    PositionedGlyph g;
    g.codepoint   = rightVowel;
    g.xOffset     = 0;
    g.yOffset     = 0;
    g.zeroAdvance = false;
    cluster.glyphs.push_back(g);
  }

  // 6. Right-half of split vowel
  if (rightSplit != 0) {
    PositionedGlyph g;
    g.codepoint   = rightSplit;
    g.xOffset     = 0;
    g.yOffset     = 0;
    g.zeroAdvance = false;
    cluster.glyphs.push_back(g);
  }

  // 7. Virama / pulli (்) — displayed above/after consonant, no advance
  if (virama != 0) {
    PositionedGlyph g;
    g.codepoint   = virama;
    g.xOffset     = -1;
    g.yOffset     = TamilOffset::VIRAMA;
    g.zeroAdvance = true;
    cluster.glyphs.push_back(g);
  }

  // 8. Anusvara (ஂ) — above the consonant
  if (anusvara != 0) {
    PositionedGlyph g;
    g.codepoint   = anusvara;
    g.xOffset     = 0;
    g.yOffset     = TamilOffset::ANUSVARA;
    g.zeroAdvance = true;
    cluster.glyphs.push_back(g);
  }

#if TAMIL_CLUSTER_DEBUG_LOGGING
  Serial.printf("[TAMIL] Cluster: %zu glyphs (base=%04X left=%04X above=%04X "
                "below=%04X right=%04X virama=%04X lsplit=%04X rsplit=%04X)\n",
                cluster.glyphs.size(), baseConsonant, leftVowel, aboveVowel,
                belowVowel, rightVowel, virama, leftSplit, rightSplit);
#endif

  return cluster;
}

}  // namespace TamilShaper
