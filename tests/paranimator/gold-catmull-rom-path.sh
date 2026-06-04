#!/usr/bin/env bash
set -e
pushd "$(dirname "$0")" >/dev/null
id batch=yes overwrite=yes savename=frame-0001.gif savedir=. librarydirs=. video=F6 @frames.par/frame-0001
id batch=yes overwrite=yes savename=frame-0002.gif savedir=. librarydirs=. video=F6 @frames.par/frame-0002
id batch=yes overwrite=yes savename=frame-0003.gif savedir=. librarydirs=. video=F6 @frames.par/frame-0003
id batch=yes overwrite=yes savename=frame-0004.gif savedir=. librarydirs=. video=F6 @frames.par/frame-0004
id batch=yes overwrite=yes savename=frame-0005.gif savedir=. librarydirs=. video=F6 @frames.par/frame-0005
id batch=yes overwrite=yes savename=frame-0006.gif savedir=. librarydirs=. video=F6 @frames.par/frame-0006
id batch=yes overwrite=yes savename=frame-0007.gif savedir=. librarydirs=. video=F6 @frames.par/frame-0007
popd >/dev/null
