#!/usr/bin/env bash
set -e
pushd "$(dirname "$0")" >/dev/null
magick \( "layers/layer-base-0001.gif" -alpha set -channel A -evaluate multiply 1 +channel \) \( "layers/layer-detail-0001.gif" -alpha set -channel A -evaluate multiply 0.5 +channel \) -compose Over -composite -background "black" -alpha remove -alpha off "frames/frame0001.png"
magick \( "layers/layer-base-0002.gif" -alpha set -channel A -evaluate multiply 1 +channel \) \( "layers/layer-detail-0002.gif" -alpha set -channel A -evaluate multiply 0.5 +channel \) -compose Over -composite -background "black" -alpha remove -alpha off "frames/frame0002.png"
magick \( "layers/layer-base-0003.gif" -alpha set -channel A -evaluate multiply 1 +channel \) \( "layers/layer-detail-0003.gif" -alpha set -channel A -evaluate multiply 0.5 +channel \) -compose Over -composite -background "black" -alpha remove -alpha off "frames/frame0003.png"
popd >/dev/null
