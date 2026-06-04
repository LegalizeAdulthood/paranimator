#!/usr/bin/env bash
set -e
pushd "$(dirname "$0")" >/dev/null
magick \( "layers/layer-base-0001.gif" -alpha set -channel A -evaluate multiply 1 +channel \) \( "layers/layer-detail-0001.gif" -alpha set -channel A -evaluate multiply 1 +channel \) -compose Screen -composite "frames/frame0001.png"
popd >/dev/null
