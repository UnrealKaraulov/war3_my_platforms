================================
GHost++ CB (Custom Build) Readme
================================


============
Introduction
============

On top of the commands that are shown in "ghost\readme.txt" there are also commands and features that are added in this custom version.
A lot of the coding has not been actually done by me, but other users from the forums.  I merely insert their code into this public release build.
The SVN Repository is located at http://ghostcb.googlecode.com

========
Commands
========
*** In battle.net (via local chat or whisper at any time):

!emote				repeats anything you say with /me in front of it.

*** In game lobby:

!normalcountdown on|off		uses either the WC3 standard countdown or GHost's traditional countdown
!start						starts the game after current number of players are pinged and spoofchecked.  The countdown will abort automatically if a player leaves.
!startn						starts the game immediately with no countdowns or delays.
!start now					alias to !startn

*** In game:


*** In admin game lobby

===================
default.cfg additions
===================
### List of countries to allow based on the two-letter country codes
### Don't separate entries with spaces or commas or anything, leave blank or comment out to allow 
### Example: approvedcountries = USCA
bot_approvedcountries = 

### Use normal countdown when set to 1 will mimic the Warcraft III game start
### Set this to 0 to set back to Ghost's normal countdown
bot_usenormalcountdown = 0

### Reload "bot_allowdownloads" after a game starts
###  0 - off (default) / 1 - on
bot_resetdownloads = 0

### Do not show who sent an admin command, if they are an admin
###  0 - show all commands / 1 - hide admin commands
###  The results (ex. "Shuffling Players") is still shown
bot_hideadmincommands = 0

### Only whisper command responses instead of sending through public chat
###  0 - normal operation / 1 - most commands are responded with a whisper
bot_whisperresponses = 0

### LAN Admins
###  0 - off (default) / 1 - LAN players will be Admins / 2 - LAN players will be Root Admins / 3 - Unspecified LAN players will be admins
lan_admins = 0

### Get LAN Admins from a list (lan_rootadmins)
###  0 - off (default) / 1 - on
###  if lan_admins = 1 & lan_getrootadmins = 1, will recognize players as admins joining from LAN only with the names specified for lan_rootadmins.
###  if lan_admins = 2 & lan_getrootadmins = 1, will recognize players as root admins joining from LAN only with the names specified for lan_rootadmins.
###  if lan_admins = 3 & lan_getrootadmins = 1, will recognize players as root admins joining from LAN with the names specified for lan_rootadmins,
###   any other players will be admins
lan_getrootadmins = 0

### LAN Admins list
### the (root) admins on LAN players
###  seperate each name with a space, e.g. lan_rootadmins = Varlock Kilranin Instinct121
lan_rootadmins = 

### HCL from Game Name
bot_hclfromgamename = 0

### Force the load-in-game feature
bot_forceloadingame = 0

########################
# CURSES CONFIGURATION #
########################

### the term buffer size / window size
term_width = 135
term_height = 52

### enable curses user interface
curses_enabled = 1

### enable split view in realm windows
curses_splitview = 0

### 0: horizontal lists, anything else: vertical lists
curses_listtype = 1

====================
map config additions
====================
### Map HCL Rotation
### If you add these three lines to a dota config,
### when autohosted the gamename will be map_gamenamewithmode as long as map_hclfromgamename = 1 and map_validmodes shows a valid mode
### it will display a random mode from map_validmodes as a replacement for $mode$ in game name
map_hclfromgamename = 1
map_validmodes = -arso -apso -rdso -sdso -aremso -apemso
map_gamenamewithmode = Dota 5v5 $mode$ !!!

========
Features
========
- improved motd.txt handling (supports variables: $OWNERNAME$, $GAMENAME$, $HCL$, $VERSION$, $USER$, $BOTNAME$)
- automatically detects the map_type as dota & sets map_matchmakingcategory to dota_elo when loading a DotA map
- blocks battle.net !say commands (ex. !say /squelch user) unless sent from a Root Admin
- improved !start command
- modified bot_ban method when set to 1.
###  if bot_banmethod = 1, GHost++ will automatically reject players using a banned name, and will print a message if it only recognizes the IP address
- admin game supports no passwords (no login required)

=======
Credits
=======
I really hope that I have been able to give credit to all those involved in developing this and other versions of GHost++.

-Varlock
-Psionic
-Lucasn
-Strilanc
-Spoofy
-Senkin
-Emmeran
-Vunutus
-disturbed_oc
-AlienLifeForm
-Instinct121
-Fire86
-Krauzi
-Damianakos
-Smaug
-Shade0o
-Destroya03
-SmokeMonster
-DantX
