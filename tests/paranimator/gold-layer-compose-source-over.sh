#!/usr/bin/env bash
set -e
pushd "$(dirname "$0")" >/dev/null
mkdir -p "layers"
id batch=yes overwrite=yes savename=layer-base-0001.gif savedir=. librarydirs=. video=F6 @frames.par/layer-base-0001
mv -f "image/layer-base-0001.gif" "layers/layer-base-0001.gif"
id batch=yes overwrite=yes savename=layer-detail-0001.gif savedir=. librarydirs=. video=F6 @frames.par/layer-detail-0001
mv -f "image/layer-detail-0001.gif" "layers/layer-detail-0001.gif"
id batch=yes overwrite=yes savename=layer-base-0002.gif savedir=. librarydirs=. video=F6 @frames.par/layer-base-0002
mv -f "image/layer-base-0002.gif" "layers/layer-base-0002.gif"
id batch=yes overwrite=yes savename=layer-detail-0002.gif savedir=. librarydirs=. video=F6 @frames.par/layer-detail-0002
mv -f "image/layer-detail-0002.gif" "layers/layer-detail-0002.gif"
id batch=yes overwrite=yes savename=layer-base-0003.gif savedir=. librarydirs=. video=F6 @frames.par/layer-base-0003
mv -f "image/layer-base-0003.gif" "layers/layer-base-0003.gif"
id batch=yes overwrite=yes savename=layer-detail-0003.gif savedir=. librarydirs=. video=F6 @frames.par/layer-detail-0003
mv -f "image/layer-detail-0003.gif" "layers/layer-detail-0003.gif"
popd >/dev/null
