#include "TamilCharacter.h"

#include <Utf8.h>

namespace TamilShaper {

TamilCharType getTamilCharType(uint32_t cp) {
  if (cp < 0x0B80 || cp > 0x0BFF) {
    return TamilCharType::NON_TAMIL;
  }

  // Independent vowels: அ-ஔ (U+0B85-U+0B94)
  if (cp >= 0x0B85 && cp <= 0x0B94) {
    return TamilCharType::INDEPENDENT_VOWEL;
  }

  // Tamil consonants: க-ஹ (U+0B95-U+0BB9)
  if (cp >= 0x0B95 && cp <= 0x0BB9) {
    return TamilCharType::CONSONANT;
  }

  // ஂ ANUSVARA (U+0B82)
  if (cp == 0x0B82) {
    return TamilCharType::ANUSVARA;
  }

  // ஃ VISARGA / AYTHAM (U+0B83)
  if (cp == 0x0B83) {
    return TamilCharType::VISARGA;
  }

  // ் VIRAMA / PULLI (U+0BCD)
  if (cp == 0x0BCD) {
    return TamilCharType::VIRAMA;
  }

  // Vowel signs
  switch (cp) {
    // Above vowel signs
    case 0x0BBF:  // VOWEL SIGN I (ி)
    case 0x0BC0:  // VOWEL SIGN II (ீ)
      return TamilCharType::ABOVE_VOWEL;

    // Below vowel signs
    case 0x0BC1:  // VOWEL SIGN U (ு)
    case 0x0BC2:  // VOWEL SIGN UU (ூ)
      return TamilCharType::BELOW_VOWEL;

    // Right-side vowel signs (displayed after/right of consonant)
    case 0x0BBE:  // VOWEL SIGN AA (ா)
    case 0x0BC3:  // VOWEL SIGN VOCALIC R
    case 0x0BC4:  // VOWEL SIGN VOCALIC RR
      return TamilCharType::RIGHT_VOWEL;

    // Left-side vowel signs (displayed before consonant, stored after in Unicode)
    case 0x0BC6:  // VOWEL SIGN E (ெ)
    case 0x0BC7:  // VOWEL SIGN EE (ே)
    case 0x0BC8:  // VOWEL SIGN AI (ை)
      return TamilCharType::LEFT_VOWEL;

    // Split (two-part) vowel signs
    case 0x0BCA:  // VOWEL SIGN O  (ொ)
    case 0x0BCB:  // VOWEL SIGN OO (ோ)
    case 0x0BCC:  // VOWEL SIGN AU (ௌ)
      return TamilCharType::SPLIT_VOWEL_LEFT;
  }

  // Tamil digits: ௦-௯ (U+0BE6-U+0BEF)
  if (cp >= 0x0BE6 && cp <= 0x0BEF) {
    return TamilCharType::TAMIL_DIGIT;
  }

  // Everything else in Tamil block is symbol/punctuation
  // (includes ௰ ௱ ௲ fraction/number signs, ௳-௺ day signs, etc.)
  return TamilCharType::TAMIL_SYMBOL;
}

bool getSplitVowelParts(uint32_t cp, uint32_t& outLeft, uint32_t& outRight) {
  switch (cp) {
    case 0x0BCA:  // ொ  = ெ (U+0BC6) + ா (U+0BBE)
      outLeft  = 0x0BC6;
      outRight = 0x0BBE;
      return true;
    case 0x0BCB:  // ோ  = ே (U+0BC7) + ா (U+0BBE)
      outLeft  = 0x0BC7;
      outRight = 0x0BBE;
      return true;
    case 0x0BCC:  // ௌ  = ெ (U+0BC6) + ௌ sign (U+0BD7)
      outLeft  = 0x0BC6;
      outRight = 0x0BD7;
      return true;
    default:
      return false;
  }
}

bool containsTamil(const char* text) {
  if (text == nullptr || *text == '\0') {
    return false;
  }

  const uint8_t* ptr = reinterpret_cast<const uint8_t*>(text);
  uint32_t cp;

  while ((cp = utf8NextCodepoint(&ptr))) {
    if (isTamilCodepoint(cp)) {
      return true;
    }
  }

  return false;
}

}  // namespace TamilShaper
