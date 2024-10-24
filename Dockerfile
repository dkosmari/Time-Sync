FROM devkitpro/devkitppc

COPY --from=ghcr.io/wiiu-env/libcurlwrapper:20240505 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libnotifications:20240426 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/wiiupluginsystem:20240505 /artifacts $DEVKITPRO

RUN apt-get install -y automake
RUN dkp-pacman -Syu --noconfirm

COPY . /project
WORKDIR /project
