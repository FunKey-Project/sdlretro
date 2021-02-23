#!/usr/bin/env sh

OPK_NAME=sdlretro-gpsp.opk
echo ${OPK_NAME}

cp ../build/src/sdlretro .
/opt/FunKey-sdk-2.0.0/bin/arm-funkey-linux-musleabihf-strip sdlretro

if [ -f cores/gpsp_libretro.so ]; then
    /opt/FunKey-sdk-2.0.0/bin/arm-funkey-linux-musleabihf-strip cores/*.so
    CORES=cores
else
	echo "ERROR: gpsp core not provided"
	exit 1
fi

# create opk
rm -f ${OPK_NAME}
mksquashfs gba.funkey-s.desktop sdlretro ../data/gba_icon.png ${CORES} ${OPK_NAME} -all-root -no-xattrs -noappend -no-exports
