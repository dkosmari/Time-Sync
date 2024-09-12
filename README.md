# Time Sync

**Time Sync** is a NTP plugin for the Wii U [Aroma](https://aroma.foryour.cafe/)
environment. It keeps the Wii U clock synchronized with Network Time Protocol services.


## Installation

You need to have the [Aroma](https://aroma.foryour.cafe/) environment installed for **Time
Sync** to work. Visit the [hacking guide](https://wiiu.hacks.guide/) for instructions on
how to install Aroma.

The [latest version of the
 plugin](https://github.com/dkosmari/Time-Sync/releases/latest/download/Time_Sync.wps) can
 be found in the [releases page](https://github.com/dkosmari/Time-Sync/releases).

 The `.wps` file should be placed on your SD card, in `sd:/wiiu/environments/aroma/plugins`.


## Usage

As long as syncing is enabled by the user, the clock will sync whenever Aroma starts, or
when the *Wii U Plugin System Config Menu* (**L** + **SELECT** + **🡻**) is closed.

**The HOME Menu and other applications might not see the updated clock until the console
is rebooted.**


### Configuration screen

Note: options can be reset back to their default value by pressing **X**.

 - **Synchronize on boot**: Synchronizes the clock on every boot. Default is **off**.

 - **Synchronize after changing configuration**: Synchronizes the clock when closing the
   configuration menu, if any change was made. Default is **on**.

 - **Show notifications**: Controls how notifications are shown while the plugin
   runs. Default is "**normal**". For more detailed notifications you can set this to
   "**verbose**".

 - **Notification duration**: How long notifications should remain on screen. Default is
   **5 s**.

 - **Time offset**: The offset of your time zone.

 - **Detect time zone offset**: Selects an online service to guess the local time zone
   offset. This is done using a free IP geolocation online service. If your time zone is
   not being detected correctly, try changing this option to a different service. Pressing
   **A** will query the selected online service, and update the **Time offset** option.

 - **Auto update time zone**: Automatically query and update the time zone offset before
   synchronizing the clock. Default is "**off**". This option is useful for automatically
   adjusting for Daylight Saving Time changes.

 - **Timeout**: How many seconds to wait for a NTP response from a server. Default is **5
   s**.

 - **Tolerance**: How many milliseconds of error will be tolerated until the clock is
   adjusted. Default is **500 ms**.

 - **Background threads**: Maximum number of background threads to use for NTP
   queries. Default is **4**, because the default server (`pool.ntp.org`) returns 4
   different addresses, so all 4 servers will be queried at the same time.

 - **NTP servers**: Shows one or more NTP servers that will be contacted for
   synchronization. Multiple servers can be specified, separated by spaces. Default is
   `pool.ntp.org`. **This option cannot be edited within the plugin, you must edit the
   JSON configuration file manually to change it.**


### Preview screen

The **Preview** screen allows for testing the plugin, without modifying the clock.

### Synchronize now!

Press **A** on this option to run the synchronization immediately, **B** to cancel the
operation.


## Build instructions

### Prerequisites

 - [devkitPro](https://devkitpro.org/) with the packages:

   - devkitPPC
   - wiiu-curl
   - wut
   - wut-tools

 - The following libraries from Wii U Plugin System and Wii U Module System (need to be
   installed manually from source):
   - [Wii U Plugin System](https://github.com/wiiu-env/WiiUPluginSystem)
   - [libcurlwrapper](https://github.com/wiiu-env/libcurlwrapper)
   - [libnotifications](https://github.com/wiiu-env/libnotifications)

If you're **building from a repository clone**, you will also need
[Automake](https://www.gnu.org/software/automake/), available as a package from your
distribution. This is **not necessary** when building the plugin from a release tarball.


### Compilation steps

If you downloaded a release tarball, you can skip step 0.

0. `./bootstrap`
1. `./configure --host=powerpc-eabi`
2. `make`

If you don't have the environment variable `DEVKITPRO` pointing to your devkitPro
installation, you will have to call `./configure` like this:

1. `./configure --host=powerpc-eabi --with-devkitpro=/path/to/devkitpro`

This is a standard Automake package. See `./configure --help` for more options.


## Build with Docker

If you have [Docker](https://www.docker.com/) set up, you can run a fully automated build
by running the `./docker-build.sh` script. When it finishes successfully, it will generate
the .wps file on the top level directory.


## About of this fork

This is a fork of [Wii U Time Sync](https://github.com/Nightkingale/Wii-U-Time-Sync) (aka
*WUTS*) meant to test significant plugin changes **before** they're polished enough to be
merged into *WUTS*.

- Do not enable both this and *WUTS* at the same time, or the Wii U clock will get
  double-adjusted, making it even more out of sync.

- This plugin uses a different config file from *WUTS*, so settings from one plugin don't
  carry over to the other.

- Version number in this plugin does not correspond to version numbers in *WUTS*.
