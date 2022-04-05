# [CS:GO] Weapon Stickers
Weapon Stickers for CS:GO, with the following features:
- Support SQLite & MySQL;
- Support Wear/Float & Rotation;
- Support Multilingual;
	
**AlliedModders:** https://forums.alliedmods.net/showthread.php?t=327078

## Requirements
- **eItems:** https://github.com/ESK0/eItems
  - **Fork eItems:** https://github.com/quasemago/eItems
- **REST in Pawn:** https://github.com/ErikMinekus/sm-ripext
- **PTaH:** https://github.com/komashchenko/PTaH
- **MultiColors:** https://github.com/Bara/Multi-Colors

## Installation
- Edit **`csgo/addons/sourcemod/configs/core.cfg`** => and set **`"FollowCSGOServerGuidelines"`** to **`"no"`**;
- Copy the folder structure to your gameserver;
- Setup **`database.cfg`**;
  - ***Example SQLite***:
```
    "csgo_weaponstickers"
    {
        "driver"        "sqlite"
        "database"      "csgo_weaponstickers"
    }
```
  - ***Example MySQL***:
```
    "csgo_weaponstickers"
    {
        "driver"        "mysql"
        "host"          "localhost"
        "database"      "mydb"
        "user"          "root"
        "pass"          ""
        //"timeout"     "0"
        "port"          "3306"
    }
```

## Preview
![1](/__git/imgs/1.jpg)
![2](/__git/imgs/2.png)
![3](/__git/imgs/3.png)
