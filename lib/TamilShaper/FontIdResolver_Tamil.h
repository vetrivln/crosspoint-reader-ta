#pragma once

#define BOOKERLY_12_FONT_ID (-20281172)
#define BOOKERLY_14_FONT_ID (438244413)
#define BOOKERLY_16_FONT_ID (1839167134)
#define BOOKERLY_18_FONT_ID (-679450484)
#define NOTOSANS_12_FONT_ID (-1983718943)
#define NOTOSANS_14_FONT_ID (-1366477686)
#define NOTOSANS_16_FONT_ID (981157477)
#define NOTOSANS_18_FONT_ID (1683195152)
#define OPENDYSLEXIC_8_FONT_ID (-853313197)
#define OPENDYSLEXIC_10_FONT_ID (963754926)
#define OPENDYSLEXIC_12_FONT_ID (858950283)
#define OPENDYSLEXIC_14_FONT_ID (1877344218)
#define UI_10_FONT_ID (22918846)
#define UI_12_FONT_ID (1635686837)
#define SMALL_FONT_ID (674098198)
#define NOTOSANSTAMIL_8_FONT_ID (1968911570)
#define NOTOSANSTAMIL_10_FONT_ID (-445406432)
#define NOTOSANSTAMIL_12_FONT_ID (925798806)
#define NOTOSANSTAMIL_14_FONT_ID (-872322185)
#define NOTOSANSTAMIL_16_FONT_ID (581363712)
#define NOTOSANSTAMIL_18_FONT_ID (1998743406)



namespace TamilFontId {

inline int forBodyFont(int bodyFontId) {
  // ── 12pt ──────────────────────────────────────────────────────────────────
  if (bodyFontId == BOOKERLY_12_FONT_ID  ||
      bodyFontId == NOTOSANS_12_FONT_ID  ||
      bodyFontId == OPENDYSLEXIC_12_FONT_ID) {
    return NOTOSANSTAMIL_12_FONT_ID;
  }

  // ── 14pt ──────────────────────────────────────────────────────────────────
  if (bodyFontId == BOOKERLY_14_FONT_ID  ||
      bodyFontId == NOTOSANS_14_FONT_ID  ||
      bodyFontId == OPENDYSLEXIC_14_FONT_ID) {
    return NOTOSANSTAMIL_14_FONT_ID;
  }

  // ── 16pt ──────────────────────────────────────────────────────────────────
  if (bodyFontId == BOOKERLY_16_FONT_ID  ||
      bodyFontId == NOTOSANS_16_FONT_ID) {
    return NOTOSANSTAMIL_16_FONT_ID;
  }

  // ── 18pt ──────────────────────────────────────────────────────────────────
  if (bodyFontId == BOOKERLY_18_FONT_ID  ||
      bodyFontId == NOTOSANS_18_FONT_ID) {
    return NOTOSANSTAMIL_18_FONT_ID;
  }

  // ── Fallback ───────────────────────────────────────────────────────────────
  return NOTOSANSTAMIL_10_FONT_ID;
}

inline bool textIsTamil(const char* utf8Text) {
  return TamilShaper::containsTamil(utf8Text);
}

}  // namespace TamilFontId
