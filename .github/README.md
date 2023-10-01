# Wii U Time Sync

A plugin that synchronizes a Wii U's clock to the Internet.

Wii U Time Sync is a Wii U homebrew plugin for the Aroma environment. It allows the
console to automatically synchronize it's date and time through the Internet, similar to
the feature found on the Nintendo Switch and other modern devices.

## Installation
A Wii U plugin file will be bundled with each release. It should be placed on your SD
card, particularly in `wiiu/environments/aroma/plugins`.
* It's important to have the Aroma environment installed for Wii U Time Sync to
  work. Please visit our [hacking guide](https://wiiu.hacks.guide/) and the [Aroma
  webpage](https://aroma.foryour.cafe/) if you would like to softmod your Wii U console.

## Usage
If the program is placed correctly on an SD card, "Time Sync" will be listed in the Aroma
environment's Wii U Plugin System Config Menu.

If "Time Sync" doesn't show up in the Wii U Plugin System Config Menu, confirm you
placed the WPS file on your SD card correctly and restart your console.

Configuration options:

* `Syncing Enabled`: Enables updating the internal clock. Otherwise the
  plugin will only report the time correction needed, but not update the clock. Default is
  `false`.
* `Show Notifications`: Shows a notification whenever Time Sync checks and/or updates the
  time.
* `Messages Duration`: Control how long the notification messages stay on screen. Default
  is 5 seconds.
* `Hours Offset`: The amount of hours to add to UTC, `0` by default.
* `Minutes Offset`: The amount of minutes to add to UTC, `0` by default.
* `Server`: Shows the current list of NTP servers that will be queried. This setting
  cannot be changed through the configuration menu, you must edit the `Time Sync.json`
  configuration file in a text editor to change this. Multiple servers can be provided,
  separated by spaces. The default is `pool.ntp.org`.

As long as syncing is enabled by the user, the clock will sync whenever Time Sync starts,
or when the plugin settings are exited.

**The changes will not be reflected in the HOME Menu and most other applications right
away, so the console will need to be rebooted for changes to be completed.**

## Credits
I hope that I am able to express my thanks as much as possible to those who made this
repository possible.
* [GaryOderNichts](https://github.com/GaryOderNichts), for writing the network connection
  code and figuring out how to set the console's date and time through homebrew (so
  basically all the functionality).
* [Maschell](https://github.com/Maschell), for his work not only with figuring out setting
  the date and time, but also his work on the Aroma environment.
* [LumaTeam](https://github.com/LumaTeam), for the time syncing code in
  [Luma3DS](https://github.com/LumaTeam/Luma3DS), which we based our code off of.
* [Lettier](https://github.com/lettier), for his work on [NTP
  Client](https://github.com/lettier/ntpclient), which in turn led to the code in both
  Luma3DS and Wii U Time Sync.
