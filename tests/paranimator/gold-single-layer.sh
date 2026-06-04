#!/usr/bin/env bash
set -e
pushd "$(dirname "$0")" >/dev/null
id batch=yes overwrite=yes savename=frame-0001.gif savedir=. librarydirs=. video=F6 @frames.par/frame-0001
id batch=yes overwrite=yes savename=frame-0002.gif savedir=. librarydirs=. video=F6 @frames.par/frame-0002
id batch=yes overwrite=yes savename=frame-0003.gif savedir=. librarydirs=. video=F6 @frames.par/frame-0003
popd >/dev/null
