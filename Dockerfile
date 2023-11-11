FROM ghcr.io/wiiu-env/devkitppc:20230621

RUN dkp-pacman -Syu --noconfirm --debug

COPY --from=ghcr.io/wiiu-env/wiiupluginsystem:20230719 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libnotifications:20230621 /artifacts $DEVKITPRO

WORKDIR project
