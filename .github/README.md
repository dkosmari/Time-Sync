# Time Sync

**Time Sync** is a Wii U homebrew plugin for the [Aroma](https://aroma.foryour.cafe/)
environment. It allows the console to automatically synchronize its date and time through
the Internet, similar to the feature found on the Nintendo Switch and other modern
devices.


## Installation

You need to have the [Aroma](https://aroma.foryour.cafe/) environment installed for **Time
Sync** to work. Please visit the [hacking guide](https://wiiu.hacks.guide/) for
instructions on how to install Aroma.

A Wii U plugin file (.wps) can be downloaded from the [releases page](releases). It should
be placed on your SD card, in `wiiu/environments/aroma/plugins`.


## Usage

As long as syncing is enabled by the user, the clock will sync whenever Aroma starts, or
when the *Wii U Plugin System Config Menu* (**L** + **SELECT** + **🡻**) is closed.

**The HOME Menu and other applications might not see the updated clock until the console
is rebooted.**


### Configuration screen

Note: options can be reset back to their default value by pressing **X**.

 - **Syncing Enabled**: Enables the plugin. Default is "**off**". That means the plugin
   will not adjust the clock until you configure it, and set this option to "yes".

 - **Show Notifications**: Controls how notifications are shown while the plugin
   runs. Default is "**yes**". For more detailed notifications you can set this to
   "verbose".

 - **Notification Duration (seconds)**: How long notifications should remain on
   screen. Default is **5** seconds.

 - **Timezone Offset**: The offset of your timezone. Use **L**/**R** to adjust by hour
   increments, **🡸**/**🡺** to adjust by minute increments.

 - **Detect Timezone (press A)**: This is not a configuration option, but a button to
   detect the timezone using [IP Geolocation from IP-API.com](https://ip-api.com). This
   will update the **Timezone Offset** option above.

 - **Auto-update Timezone Offset**: automatically update the timezone before running a
   synchronization task. Default is "**no**".

 - **Tolerance (milliseconds)**: How many milliseconds of error will be tolerated until
   the clock is adjusted. Default is **500** ms.

 - **Server**: Shows one or more NTP servers that will be contacted for
   synchronization. Multiple servers can be specified, separated by spaces. Default is
   `pool.ntp.org`. This option cannot be edited within the plugin, you must edit the JSON
   configuration file manually to change it.

 - **Background threads**: Maximum number of background threads to use for NTP
   queries. Default is **4**, because the default server (`pool.ntp.org`) returns 4
   different addresses, so all 4 servers will be queried at the same time.


### Preview screen

The **Preview** screen allows for testing the plugin, without modifying the clock.


## Credits

I hope that I am able to express my thanks as much as possible to those who made this
repository possible.

 - [dkosmari](https://github.com/dkosmari), for his excellent refactoring of Wii U Time
   Sync, being used as our codebase ever since the release of v2.0.0.

 - [GaryOderNichts](https://github.com/GaryOderNichts), for writing the network connection
   code and figuring out how to set the console's date and time through homebrew (so
   basically all the functionality).

 - [Maschell](https://github.com/Maschell), for his work not only with figuring out
   setting the date and time, but also his work on the Aroma environment.

 - [LumaTeam](https://github.com/LumaTeam), for the time syncing code in
   [Luma3DS](https://github.com/LumaTeam/Luma3DS), which we based our code off of.

 - [Lettier](https://github.com/lettier), for his work on [NTP
   Client](https://github.com/lettier/ntpclient), which in turn led to the code in both
   Luma3DS and Wii U Time Sync.
