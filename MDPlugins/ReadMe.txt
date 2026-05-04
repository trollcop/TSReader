**************************************************************************
*  Many special thanks go to NoOneImportant for his wisdom and knowledge *
*  and most importantly his willingness to share.                        *
**************************************************************************
*                        A little EmuNation Humor:                       *
*------------------------------------------------------------------------*
* You must pray to emunation every night so you have TV every morning    *
* You must hate the infidel "nasa plugin" with all your strength         *
* Since "nasa plugin" will never give you the pleasure i can give you    *
* You shall not install any other plugin before me                       *
* BloatWare will always leaving you wanting                              *
*                                                                        *
* And now a reading from the book of EmuNation:                          *
*             And lo, did the Lord sayeth on the sixth day,              *       
*             'Let there be TV', and the people rejoiced.                *
*             And lo, did he instruct that this TV be watched            *
*             for free, so that the faithful may worship on their        *
*             big screen TVs, and needeth not Satan's CAM.               *
*             And so we releaseth this Emulator in his name, and pray    *
*             that he doth deliver us the next generation of the         *
*             sacred signal from the heavens.                            *
*             Amen.                                                      *
**************************************************************************

VC++ 2005 Redistribution pack for Microsoft has to be installed... 
h**p://www.microsoft.com/downloads/details.aspx?displaylang=en&FamilyID=32BC1BEE-A3F9-4C13-9C99-220B62A191EE 

File locations:
---------------
*** ProgDVB & MyTheatre & MediaPortal TV Server (TV3) ***
  {root} 
    pthreadVC2.dll   {VC++ support file}

  {root}\Plugins 
    EmuNation.dll
    EmuNation.ini
    Softcam.key      {see sample for required keys}

  {root}\Plugins\Nagra    (Required files to use NoOneImportant's AU}
    Rom102.bin       {ROM 102 bin file - 98304 bytes}
    EEP01_102.bin    {Dish rev10B - 18432 bytes)
    EEP08_102.bin    {Bev  rev147 - 18432 bytes)

Please make sure the names are EXACTLY as they are shown above for your provider! 
The ROM102.bin has an md5sum ('md5sum ROM102.bin') of: f05c2feeec184c6c5f3f327a90712b09 

Additionally for Mediaportal TV Server you must install and register Agarwals MDAPI filter.
You can find the filter, as well as instructions on how to install it, on DVBN

Tested with the following:
-------------------------
Alt-DVB
DVB Dream
MediaPortal TVServer with Agarwals MDAPI Filter (DVB-S only)
MyTheatre
ProgDVB
RitzDVB

Information regarding EmuNation 2.0.0.4
========================================
Prettier fix for same problem 2.0.0.3 addressed :)
Thanks to KillerCoder for this one.
Again, if someone with artistic aptitude wants to create a better monitor graphic just let me know :)

Information regarding EmuNation 2.0.0.3
========================================
Proper register 16 handling.  Thanks to "d" fpr this.
Added a graphic to the monitor dialog (I was jealous because vplug had one :) ).
If someone with artistic aptitude wants to create a better one just let me know :)

Information regarding EmuNation 2.0.0.2
========================================
KillerCoders $16 changes to nagra handling.

Information regarding EmuNation 2.0.0.1
========================================
Changes for "Kabel Deutschland".  Tt makes provider 1101 use the same MECM 
handling code as provider 0501.  Moved some existing code around to avoid duplication.

Information regarding EmuNation 2.0.0.0
========================================
Provider prioritization included.  See providers.conf file for how to configure it.
For NA people, providers.conf.NA is included.  Just rename it to providers.conf.

Information regarding EmuNation 1.5.0.1
========================================
Creation of emunation.txt debug file disabled.
Premier now works ... enjoy :) (make sure North America flag is set properly)


Information regarding EmuNation 1.5.0.0
========================================
New flag added to INI file:
NorthAmerica=1        ; (1=North America any other value means not NA)
This flag helps distinguish location for EMM (AutoECM) processing. Defaults to North America
and can be set via the Emunation menu. 

Information regarding EmuNation 1.4.3.5
========================================
Nano 0x16 added.

Information regarding EmuNation 1.4.3.4
========================================
KillerCoders map additions.
Fixed memory leak in 1.4.3.0.

Information regarding EmuNation 1.4.3.1, 1.4.3.2, amd 1.4.3.3
========================================
Bad releases.  Broken for Dish and MT.

Information regarding EmuNation 1.4.3.0
========================================
Map 0F fix as per sasc-ng patch by KillerCoder.
E3 nanos logged to e3.log (conditional based on new menu option)

Information regarding EmuNation 1.4.2.9
========================================
Note:  1.4.2.8 was a interim patch fix and was never a full release.
AU fix as per sasc-ng patch by KillerCoder.
Made 'Beep On New Key' option work


Information regarding EmuNation 1.4.2.7
========================================
Minor AU fix
SmileX745 AU has been abandoned (NoOneImportant's works fine and mine was to hard to maintain)
 AU display options in console:
   (0) Basic Information
   (1) Debug Information
   (2) Verbose Information
ALL Logs Stored in Daily Path Folder
Logging Option for Monitor Messages
New ini File Format (Menu Options under that section name)

Information regarding EmuNation 1.4.2.6
========================================
Ritz Support (Many thanks to: vahid and remote for their help)
Plugin saves Monitor and Console positions

Information regarding EmuNation 1.4.2.5
========================================
This version is based on "vdr-sc-0.6.0" 
Added ability to turn "Beep On New Key" on/off from menu
Added ability to turn "Process ECM Packets" on/off from menu
Added ability to turn "Process EMM Packets" on/off from menu
Minor operation and GUI changes made.
Added Map-57 Support

Information regarding EmuNation 1.4.2.4
========================================
Fixed "Keep Old Keys" on/off from menu. It toggled EMM-G decrypted packet logging on/off

Information regarding EmuNation 1.4.2.3
========================================
Fixed Provider 0905 ECM packet Decryption Failure
Fixed ability to turn EMM-G decrypted packet logging on/off
Added ability to turn "Keep Old Keys" on/off from menu
Other minor menu changes made.

Information regarding EmuNation 1.4.2.2
========================================
Inverse CW's fix for:
  Provider 0501 - PW 
  Provider 1101 - KD 23,5 Kabel 
  Provider 1102 - KD ISH 

Information regarding EmuNation 1.4.2.1
========================================
This version is based on "vdr-sc-0.5.12" 
In addition, this also uses a totally new stream parser that increases the number of 
packets the plugin can process. Through testing it was determined that the previous 
method only sent ~15% for post processing. We improved this rate to over 98% of all 
available packets. With some additional error correction, we also corrected 85% of 
the "bad-packet" error messages. We also now reveal that EmuNation will process EMM-s
packets. (You can use your own imagination what this feature can be used for. The 
softcam.key file shows the format used for this feature.) Other minor menu, operation
and GUI changes made. -Enjoy- 
(oh yeah, I still have not verified all the $B1 sub cmds... maybe someday)

Information regarding EmuNation 1.4.2.0
========================================
SmileX745 AU is working once again... Well, I still need to verify all the nano's but
at least we have options again.

Information regarding EmuNation 1.4.1.0
========================================
Finally no more VS image file required...

Killercoder created this patch using the Dreambox emu code that was reverse engineered 
from NoOneImportants patch. It currently only supports map 3b, so we'll have to go back 
to his original patch if map 57 comes back. 

That said, it works VERY well for the streams currently in use for Bev and Dish, 
and seems to have better performance/stability. 


-----------------------
1.4.2.7 ini file specs:
-----------------------
[EmuNation]
AuxEmu=1              ;Emulate Aux MAPROM (0=Use AuxServer for MAP Calls / 1=Use Cardless fix) 
NADCWTweak=1          ;NorthAmerican DCW Ordering Tweak--Prevents video freezing on channel changes (0=off / 1=on)
S_K_location=         ;Alternate location of "softcam.key" file
UseCaCache=0          ;(0=update Softcam.key / 1=update Ca.Cache )

[Menu Options]
Active=1              ;Plugin enabled (0=off / 1=on)
BeepOnKeyFind=1       ;Beep On New Key (0=off / 1=on)
HideBadData=1         ;Do not display bad messages in monitor (0=off / 1=on)
KeepOldKeys=0         ;(0=just keep new key / 1=old key key and comment) used with Softcam.key file
LoadMonitorOnStart=0  ;Load Monitor on Startup (0=off / 1=on)
ProcessECM=1          ;Process ECM Packets (0=off / 1=on)
ProcessEMM=1          ;Process EMM Packets (0=off / 1=on)
ProcessEmmS=0         ;If EMM-S keys exist, send for processing (0=off / 1=on)
ShowConsole=0         ;Show Debug Console on Startup (0=off / 1=on)
ShowDisassembly=0     ;(0=Basic / 1=Debug / 2=Verbose)
UseMaxParser=1        ;(0=Use old method / 1=Use New Parser)

[Logger]
LogPath=              ;Path to use for ALL logging (default: {root}\Logs)
LogDCW=0              ;Log DCWs to file (0=off / 1=on)
LogECMBin=0           ;Log ECMs as binary data to file (0=off / 1=on)
LogEMMBin=0           ;Log EMMs as binary data to file (0=off / 1=on)
LogECMText=0          ;Log ECMs as hex data to file (0=off / 1=on)
LogEMMText=0          ;Log EMMs as hex data to file (0=off / 1=on)
LogEMM-G=0            ;Log Decrypted EMM-G to file (0=off / 1=on)
LogEMM-S=0            ;LogEMMs= 0=no logging / 1=single file / 2= two files (group / cam spec.) / 3=by camid (default =0)
LogRawEMM-S=0         ;If logging... Log Encrypted EMM-S Packets (0=off / 1=on)
LogConsole=0          ;Log Console Messages to file (0=off / 1=on)
LogMonitor=0          ;Log Monitor Messages to file (0=off / 1=on)

[0101]
emm_pid_hex=0120      ;Hex value of PID for this provider
EMMOnChannelChange=1  ;Auto start EMM filter on channel change (1=Start autoroll every channel change/0=Start autoroll only when keys are invalid) 
EMM_off_OnKeyFind=0   ;Stop EMM filter on key find (0=off / 1=on)
 
[0901]
emm_pid_hex=00F9      ;Hex value of PID for this provider
EMMOnChannelChange=1  ;Auto start EMM filter on channel change (1=Start autoroll every channel change/0=Start autoroll only when keys are invalid) 
EMM_off_OnKeyFind=0   ;Stop EMM filter on key find (0=off / 1=on)
 
[AuxServer]
Address=127.0.0.1     ;AuxServer server address (Default:127.0.0.1) 
Port=7777             ;AuxServer port (Default:7777) 
Password=auxserver    ;AuxServer Password (Default:auxserver) 
