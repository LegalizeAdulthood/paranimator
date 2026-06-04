#!/usr/bin/env bash
set -e
pushd "$(dirname "$0")" >/dev/null
mkdir -p "layers"
id batch=yes overwrite=yes savename=layer-base-0001.gif savedir=. librarydirs=. video=F6 @frames.par/layer-base-0001
mv -f "image/layer-base-0001.gif" "layers/layer-base-0001.gif"
id batch=yes overwrite=yes savename=layer-detail-0001.gif savedir=. librarydirs=. video=F6 @frames.par/layer-detail-0001
mv -f "image/layer-detail-0001.gif" "layers/layer-detail-0001.gif"
popd >/dev/null
