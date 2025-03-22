#!/bin/bash

PLUGIN_NAME="time-sync"
IMAGE="${PLUGIN_NAME}-image"
CONTAINER="${PLUGIN_NAME}-container"


cleanup()
{
    echo "Cleaning up Docker..."
    if test x$CONTAINER != x
    then
        docker container rm --force $CONTAINER
    fi
    if test x$IMAGE != x
    then
        docker image rm --force $IMAGE
    fi
    echo "You may also want to run 'docker system prune --force' to delete Docker's caches."
    exit $1
}
trap cleanup INT TERM


docker build --tag $IMAGE . || cleanup 1

ARGS="--tty --interactive --name $CONTAINER $IMAGE"
docker run $ARGS sh -c "./bootstrap && ./configure --host=powerpc-eabi CXXFLAGS='-Os -ffunction-sections -fipa-pta -flto -Wno-odr' AR=powerpc-eabi-gcc-ar RANLIB=powerpc-eabi-gcc-ranlib && make" || cleanup 2
echo "Compilation finished."

# Copy the wps file out.
docker cp "$CONTAINER:/project/time-sync.wps" .

cleanup 0
