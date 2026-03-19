#pragma once

/**
 * TamilShaper — Tamil script grapheme cluster builder for e-ink rendering.
 *
 * Public API header. Include this from the rendering pipeline.
 *
 * Usage:
 *
 *   #include <TamilShaper/TamilShaper.h>
 *
 *   if (TamilShaper::containsTamil(myUtf8Text)) {
 *     auto clusters = TamilShaper::TamilClusterBuilder::buildClusters(myUtf8Text);
 *     for (auto& cluster : clusters) {
 *       for (auto& glyph : cluster.glyphs) {
 *         // render glyph.codepoint at (cursorX + glyph.xOffset, baseY + glyph.yOffset)
 *         if (!glyph.zeroAdvance) cursorX += advanceWidthFor(glyph.codepoint);
 *       }
 *     }
 *   }
 */

#include "TamilCharacter.h"
#include "TamilCluster.h"
#include "TamilClusterBuilder.h"
