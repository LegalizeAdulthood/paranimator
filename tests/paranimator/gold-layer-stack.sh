#!/usr/bin/env bash
set -e
pushd "$(dirname "$0")" >/dev/null
id batch=yes overwrite=yes savename=layer-base-0001.gif savedir=. librarydirs=. video=F6 @frames.par/layer-base-0001
id batch=yes overwrite=yes savename=layer-detail-0001.gif savedir=. librarydirs=. video=F6 @frames.par/layer-detail-0001
id batch=yes overwrite=yes savename=layer-base-0002.gif savedir=. librarydirs=. video=F6 @frames.par/layer-base-0002
id batch=yes overwrite=yes savename=layer-detail-0002.gif savedir=. librarydirs=. video=F6 @frames.par/layer-detail-0002
id batch=yes overwrite=yes savename=layer-base-0003.gif savedir=. librarydirs=. video=F6 @frames.par/layer-base-0003
id batch=yes overwrite=yes savename=layer-detail-0003.gif savedir=. librarydirs=. video=F6 @frames.par/layer-detail-0003
popd >/dev/null
