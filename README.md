# [CS:GO] Weapon Stickers
Weapon Stickers for CS:GO: https://forums.alliedmods.net/showthread.php?t=327078

## Description
There's not much to say, self-explanatory title. This plugin remained private for a long time, but it was recently leaked by people of bad trust and I decided to publish it.

## ConVars:
You can modify the ConVars below in **cfg/sourcemod/csgo_weaponstickers.cfg** (auto generate)
- **sm_weaponstickers_flag** - Specifies the required flag.
  <br>EG: 'a' for reserved slot.
- **sm_weaponstickers_overrideview** - Specifies whether the plugin will override the weapon view.
  <br>PS: To use your own skins, you must disable this cvar.
- **sm_weaponstickers_reusetime** - Specifies how many seconds it will be necessary to wait to update the stickers again.
- **sm_weaponstickers_updateviewmodel** - Specifies whether the view model will be updated when changing stickers.
  <br>PS: This is necessary so that the player does not need to switch weapons to update the sticker.

## Dependencies:
- eItems: https://github.com/ESK0/eItems
- PTaH: https://github.com/komashchenko/PTaH
- Multi-Colors: https://github.com/Bara/Multi-Colors
- SourceScramble [Win Only]: https://github.com/nosoop/SMExt-SourceScramble

## Installation
- Edit **csgo/addons/sourcemod/configs/core.cfg** => and set **"FollowCSGOServerGuidelines"** to **"no"**.
- Copy the folder structure to your gameserver.
- Setup database. Example:
```    "csgo_weaponstickers"
    {
        "driver"            "mysql"
        "host"                "localhost"
        "database"            "mydb"
        "user"                "root"
        "pass"                ""
        //"timeout"            "0"
        "port"                "3306"
    }
```
## Preview:
![1](/__git/imgs/1.jpg)
![2](/__git/imgs/2.png)
![3](/__git/imgs/3.png)
