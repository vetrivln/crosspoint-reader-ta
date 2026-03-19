#!/bin/bash

NOTOSANSTAMIL_FONT_SIZES=(8 10 12 14 16 18)
NOTOSANSTAMIL_FONT_STYLES=("Regular" "Bold")

for size in ${NOTOSANSTAMIL_FONT_SIZES[@]}; do
  for style in ${NOTOSANSTAMIL_FONT_STYLES[@]}; do
    font_name="notosanstamil_${size}_$(echo $style | tr '[:upper:]' '[:lower:]')"
    font_path="../builtinFonts/source/NotoSansTamil/NotoSansTamil-${style}.ttf"
    output_path="../builtinFonts/${font_name}.h"
    python fontconvert.py $font_name $size $font_path \
      --2bit --compress \
      --additional-intervals 0x0B80,0x0BFF \
      > $output_path
    echo "Generated $output_path"
  done
done
