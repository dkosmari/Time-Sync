# Time Sync

A plugin that synchronizes a Wii U's clock to the NTP servers on the internet.

Time Sync is a Wii U homebrew plugin for the Aroma environment. It allows the console to
automatically synchronize its date and time through the Internet, similar to the feature
found on the Nintendo Switch and other modern devices.

## Installation
A Wii U plugin file (.wps) can be downloaded from the [Releases page](releases). It should
be placed on your SD card, particularly in `wiiu/environments/aroma/plugins`.
* You need to have the Aroma environment installed for Time Sync to work. Please visit the
  [hacking guide](https://wiiu.hacks.guide/) and the [Aroma
  webpage](https://aroma.foryour.cafe/) if you would like to softmod your Wii U console.

## Usage
If the plugin is placed correctly on an SD card, "Time Sync" will be listed in the Aroma
environment's Wii U Plugin System Config Menu.

If "Time Sync" doesn't show up in the Wii U Plugin System Config Menu, confirm you
placed the .wps file on your SD card correctly and restart your console.

Configuration options:

* `Syncing Enabled`: Enables the plugin. The default is `false`.
* `Show Notifications`: Shows a notification whenever the plugin checks and/or updates the
  time. The default is `true`.
* `Notification Duration`: Controls how long the notification messages stay on
  screen. Default is `5 seconds`.
* `Hours Offset`: The amount of hours to add to UTC to match your local time, `0` by
  default.
* `Minutes Offset`: The amount of minutes to add to UTC to match your local time, `0` by
  default.
* `Detect Timezone`: press A to use an IP geolocation service (ip-api.com) to estimate
  your timezone. This will automatically modify the `Hours Offset` and `Minutes Offset`,
  and show the name of the detected timezone.
* `Tolerance`: Don't alter the clock when the difference is less than this. Default is
  `250 ms`.
* `NTP Servers`: Shows the current list of NTP servers that will be queried. This setting
  cannot be changed through the configuration menu. If you want to use a different server,
  or add more servers, you must edit the `Time Sync.json` configuration file in a text
  editor. Multiple servers can be provided, separated by spaces (or commas, or
  semicolons.) The default is `pool.ntp.org`.

As long as syncing is enabled by the user, **the clock will sync whenever the plugin starts,
or when the plugin settings are exited**.

The changes **will not appear** in the HOME Menu and most other applications right away,
so the console will need to be **rebooted** for the changes to be visible by the system.

There's also a **Preview** section, where you can just press A to query the NTP servers,
without making any modifications to the local clock.

## Credits
I hope that I am able to express my thanks as much as possible to those who made this
repository possible.
* [GaryOderNichts](https://github.com/GaryOderNichts), for writing the network connection
  code and figuring out how to set the console's date and time through homebrew (so
  basically all the functionality).
* [Maschell](https://github.com/Maschell), for his work not only with figuring out setting
  the date and time, but also his work on the Aroma environment.

