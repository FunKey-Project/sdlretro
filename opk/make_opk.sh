#!/usr/bin/env sh

if [ -n "$FULL" ]; then
    OPK_NAME=sdlretro-full.opk
else
    OPK_NAME=sdlretro.opk
fi

rm -f $OPK_NAME

echo ${OPK_NAME}

cp ../build/src/sdlretro .
/opt/FunKey-sdk-2.0.0/bin/arm-funkey-linux-musleabihf-strip sdlretro

if [ -n "$FULL" ] && [ -d cores ]; then
    /opt/FunKey-sdk-2.0.0/bin/arm-funkey-linux-musleabihf-strip cores/*.so
    CORES=cores
fi

# create opk
rm -f ${OPK_NAME}
mksquashfs default.funkey-s.desktop sdlretro ../data/sdlretro_icon.png ${CORES} ${OPK_NAME} -all-root -no-xattrs -noappend -no-exports
