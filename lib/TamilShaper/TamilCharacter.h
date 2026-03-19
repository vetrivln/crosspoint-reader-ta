#pragma once

#include <cstdint>

namespace TamilShaper {

// Tamil character types for positioning and cluster formation
enum class TamilCharType : uint8_t {
  NON_TAMIL,          // Not a Tamil character
  INDEPENDENT_VOWEL,  // Independent vowel (அ ஆ இ ஈ உ ஊ எ ஏ ஐ ஒ ஓ ஔ)
  CONSONANT,          // Base consonant (க ங ச ஞ ட ண த ந ப ம ய ர ல வ ழ ள ற ன ஷ ஸ ஹ)
  ABOVE_VOWEL,        // Vowel sign above consonant (ி U+0BBF, ீ U+0BC0)
  BELOW_VOWEL,        // Vowel sign below consonant (ு U+0BC1, ூ U+0BC2)
  RIGHT_VOWEL,        // Vowel sign to right of consonant (ா U+0BBE, ர்  ழ் etc.)
  LEFT_VOWEL,         // Vowel sign that appears LEFT of consonant (ெ U+0BC6, ே U+0BC7, ை U+0BC8)
  SPLIT_VOWEL_LEFT,   // Left part of a two-part vowel sign (ொ ோ ௌ)
  SPLIT_VOWEL_RIGHT,  // Right part of a two-part vowel sign (paired with LEFT)
  VIRAMA,             // Pulli/virama (் U+0BCD) - suppresses inherent vowel
  ANUSVARA,           // Tamil anusvara (ஂ U+0B82)
  VISARGA,            // Tamil visarga / aytham (ஃ U+0B83)
  TAMIL_DIGIT,        // Tamil digits (௦-௯ U+0BE6-U+0BEF)
  TAMIL_SYMBOL,       // Tamil punctuation and symbols
};

// Check if a codepoint is in the Tamil Unicode block
inline bool isTamilCodepoint(uint32_t cp) { return cp >= 0x0B80 && cp <= 0x0BFF; }

// Check if codepoint is a Tamil consonant (can be a cluster base)
inline bool isTamilConsonant(uint32_t cp) {
  // Main consonants: க-ஹ
  return (cp >= 0x0B95 && cp <= 0x0BB9) ||
         cp == 0x0B9C;  // ஜ (ja) - sometimes excluded but present in Unicode
}

// Check if codepoint is an independent vowel
inline bool isTamilIndependentVowel(uint32_t cp) {
  return cp >= 0x0B85 && cp <= 0x0B94;
}

// Check if codepoint is the virama/pulli (consonant cluster marker)
inline bool isTamilVirama(uint32_t cp) {
  return cp == 0x0BCD;
}

// Check if codepoint is a Tamil above vowel sign
inline bool isTamilAboveVowel(uint32_t cp) {
  return cp == 0x0BBF ||  // VOWEL SIGN I (ி)
         cp == 0x0BC0;    // VOWEL SIGN II (ீ)
}

// Check if codepoint is a Tamil below vowel sign
inline bool isTamilBelowVowel(uint32_t cp) {
  return cp == 0x0BC1 ||  // VOWEL SIGN U (ு)
         cp == 0x0BC2;    // VOWEL SIGN UU (ூ)
}

// Check if codepoint is a right-side vowel sign
inline bool isTamilRightVowel(uint32_t cp) {
  return cp == 0x0BBE ||  // VOWEL SIGN AA (ா)
         cp == 0x0BC3 ||  // VOWEL SIGN VOCALIC R (rare)
         cp == 0x0BC4;    // VOWEL SIGN VOCALIC RR (rare)
}

// Left-side vowel signs (appear before the consonant visually)
inline bool isTamilLeftVowel(uint32_t cp) {
  return cp == 0x0BC6 ||  // VOWEL SIGN E (ெ)
         cp == 0x0BC7 ||  // VOWEL SIGN EE (ே)
         cp == 0x0BC8;    // VOWEL SIGN AI (ை)
}

// Two-part (split) vowel signs that appear on both sides of the consonant
// Left part stored here; right part is always ா (U+0BBE)
inline bool isTamilSplitVowel(uint32_t cp) {
  return cp == 0x0BCA ||  // VOWEL SIGN O  (ொ = ெ + ா)
         cp == 0x0BCB ||  // VOWEL SIGN OO (ோ = ே + ா)
         cp == 0x0BCC;    // VOWEL SIGN AU (ௌ = ெ + ௌ)
}

// Get the character type for a Tamil codepoint
TamilCharType getTamilCharType(uint32_t cp);

// Check if text contains any Tamil codepoints
bool containsTamil(const char* text);

// For split vowels, return the left-part codepoint to display before consonant
// and the right-part codepoint to display after consonant.
// Returns false if cp is not a split vowel.
bool getSplitVowelParts(uint32_t cp, uint32_t& outLeft, uint32_t& outRight);

}  // namespace TamilShaper
