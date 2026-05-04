THIS SOFTWARE IS CHARITYWARE
If you use it regularly, please consider sending some money
to the Charity Organization of your choice. I suggest 
UNICEF.


FENRIR Plugin
*************

-----------------
-  Description  -
-----------------

A) Fenrir is a plugin for Multidec-API compliant application,
like Multidec, ProgDVB, MyTheatre and many others.
It now has also a native DVBCore(MyTheatre) interface.

Its purpose is to find new keys (auto-update) on some crypto
systems: currently the following are supported:

- viaccess1
- irdeto1/betacrypt1
- seca1 (also with P+S MK)
- conax (untested since golden MK killed)
- nagravision1 *

* Depending on the version of Fenrir, Nagravision1 is supported
either internally or with the help of the package CAID1800 (not
provided!). The idea is to allow easier update of Nagra code by
the CAID1800 author. For the version of Fenrir (called External),
without Nagra, the file CAID1800.DLL is expected in the same
directory as fenrir.dll, and the subdirectory NAGRA must be there too
(with the data files described further below).

It has a Monitor window showing the new found keys...
Monitor has a button to add custom EMM PID, but it's not kept:
if switch to another transponder, you'll have to enter it again.

B) Fenrir is also a stand-alone Auto-Update library that you could
use independantly from Multidec-API. You could integrate it
in your own application/plugin. In this case, it's advise that
you rename fenrir.dll to another extension, to prevent it being
loaded as a plugin.

For MyTheatre and possibly some others DVBCore programs:
you can either copy the files into MDPlugins and use MDWrapper, 
*or* put them in Plugins: in both cases you won't get any OSD messages.

Canonic directory structure:
<whatever>\
	\Fenrir.dll
	\fenrir.ini
	\caid1800.dll	(optional)
in PrvPath (default is same as Fenrir.dll)
	\seca.prv
	\ird-beta.prv
	\yankse.prv	(optional)
in KIDPath (default is same as Fenrir.dll directory)
	\*.kid		
	\*.sci		
	\NAGRA\		
	\NAGRA\NagraKey.txt
	\NAGRA\EEP*.bin
	\NAGRA\ROM*.bin
	\NAGRA\RAM*.bin

The CAID1800 package is expected to contain the data files in \NAGRA\ directory.

REMARK: the CAID1800 package is not yet available, and for now, all the nagra stuff
is done internally by Fenrir.

--------------------
-  Files required  -
--------------------

To work, Fenrir expects some files:
* fenrir.ini in the same directory as fenrir.dll
  [Fenrir]
  Disabled=0|1;	; if =1, no background logging is done, default=0 (active)
  Log= 0|1	; Log some info to fenrir.log (default=0)
  AUSeca=0|1	; Background AU on Seca (1=default)
  AUViaccess=0|1; ditto for Viaccess
  AUNagra=0|1	; ditto for Nagravision
  AUIrdeto=0|1	; ditto for Irdeto/Betacrypt
  AUConax=0|1	; ditto for Conax

  [Path]
  KID=.		; location of directory containing the *.kid (or .sci) files
  Key=.		; location where the new found keys will be written
  Prv=.		; diretcory containing the yankse.prv and/or seca.prv, viaccess.prv
		; ird-beta.prv files (providers names)
		; the default for all these 3 paths is . (same directory as the .dll)

---------------------
- Location of Files -
---------------------

Path for KID.Key/Prv files (and nagra) can be customized as shown abobe by using the
KID/Key/Prv=<path> in the [Path] section of Fenrir.ini.

Registry can also be used (INI values takes precedence)
HKEY_CURRENT_USER\Software\BlueRiver\Fenrir
	String values: 
		KIDPath=... path
		KeyPath=... path
		PrvPath=... path
All are optional.
If no values are found (either in registry or INI), the Fenrir.dll directory is assumed.
You can use the provided "Fenrir.reg" to create the registry keys, then edit them to your needs.
The default is ".", meaning current directory.

Yankse >=1.07 can use the same Registry Key (KeyPath) to find the update keys :-)

This way, you can have onle a single copy of your MK and keys for several programs :-))


Syntax of KID/SCI files
-----------------------

All the KID files are subject to be modified to reflect the auto-update of keys

****************
* IRD-BETA.KID *
****************

; *******  MAXIMAL 256 Karten ********
;
; AAAAAA               = Hex-Serial ( 3 Byte )
; BBBBBBBBBBBBBBBBBBBB = Hex Master Key ( 10 Byte)
;
; CC                   = Provider ( 1 Byte )
; DDDDDD               = Provider ID ( 3 Bytes )
; EEEEEEEEEEEEEEEE     = Plain Master Key ( 8 Byte)
;
AAAAAA BBBBBBBBBBBBBBBBBBBB CC DDDDDD EEEEEEEEEEEEEEEE;Eigner Kommentar;LastUpdate;ggf passende Idents bei Provider ID = 000000
;

Only the Provider, Provider ID and PMK are really useful.
It's only line per card.

************
* SECA.KID *
************

; List of known SECA-MasterKeys ( KEY 01)
; Can be edited !! Written by Fenrir/MDLoggen
; *******  256 Cards Max ********
; XXXX at start of line = Provider-Ident
; YYYYYYYY = Shared-Address ( the PPUA are the first 3 bytes ;-)
; ZZZZZZZZZZZZZZZZ The Key 01.
; ZZZZZZZZZZZZZZZZzzzzzzzzzzzzzzzz The Key 01 Primary+Secondary.
;
XXXX YYYYYYYY ZZZZZZZZZZZZZZZZ;ProviderName;LastUpdate
XXXX YYYYYYYY ZZZZZZZZZZZZZZZZzzzzzzzzzzzzzzzz;ProviderName;LastUpdate

It's one line per card.

*************
* CONAX.KID *
*************

*** CanalDigital ***

IDENT=000000
UA=UUUUUUUUUUUUUU
SA=SSSSSSSSSSSSSS
MOD00=MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM ; 64 bytes=128 hex characters
EXP00=EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
MOD10=MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
EXP10=EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE

*** Other Provider ***

IDENT=000001
UA=UUUUUUUUUUUUUU
SA=SSSSSSSSSSSSSS
MOD00=MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM ; 64 bytes=128 hex characters
EXP00=EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
MOD10=MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
EXP10=EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE

Only Keys (Exp/Mod) 00 and 10 are useful, but unfortunately nobody knows them anymore :(


****************
* VIACCESS.SCI *
****************

***** Provider Name ******

IDENT= FFF500
UA= 00UUUUUUUU
SA= 00SSSSSS
MK0= 0000000000000000
MK1= 1111111111111111
MK2= 2222222222222222
MK3= 3333333333333333
MK4= 4444444444444444
MK5= 5555555555555555
MK6= 6666666666666666
MK7= 7777777777777777


***** Other provider *********

IDENT= 00C400
...

You can specify the valid MKs only, you don't have to enter MK2 if
MK2 is empty.


****************
* NagraKey.txt *
****************
This file must be located in a "nagra" subdirectory (relative to the KID Path)

[0001 - Dish Networks]
Universal:
N1[2]=
E1[2]=
N2[0]=
VK[0]=
ROM10:
N2[1]=
ROM11:
N1[0]=
E1[0]=
N1[1]=
E1[1]=
N1[2]=
E1[2]=
N2[1]=
VK[1]=

Fill the fields with the key values matching the ROM version of the provider,
only PK2 (to all cards) are useful.
To work, Nagravision, needs also several data files called EEP*.bin, ROM*.bin
and RAM*.bin (for different card versions), located in the "Nagra" subdirectory.


--------------------------------------------------------------------------------

Files written with new keys
***************************

seca.key, viaccess.key, nagra.key, ird-beta.key, conax.key according to the well-known
syntax of these files (usable by Yankse)
these are written to the directory specified by the KeyPath.


  /////////
 // FAQ //
/////////

Q: Fenrir doesn't log anything in MyTheatre (RitzDVB) ?
A: For TV Applications based on DVBCore (MyTheatre, RitzDVB...), make sure you're using
   the right MDWrapper version: I suggest 1.22, the stock version (1.10 ?) does not 
   work right.

Q: Why isn't Fenrir a DVBCore plugin ?
A: Updated: yes it is now !

Q: Where do I get KID files ?
A: You extract the Masters keys from you own card (remember the disclaimer: you need to have
   a subscribtion ;), or from a friend. In a general manner, you don't beg for such keys
   on public forums (and not to me either!)

(more to come ? ;-)

  ////////////////////
 / Revision history /
////////////////////

V 1.0.1.1  03-jun-29	First public release
	Major features: * First release ;-)


V 1.0.2.1  03-jul-21	
	Minor features: * KID/Key/Prv paths customizable in Ini and registry
			* HispaDVB OSD-aware
	Bugfix:		* correction in nagra code for Dish

V 1.0.3.1  03-sep-08
	Minor features: * DVBCore interface (no OSD yet) thanks to Saar :-))
			* updated the Nagra emulator for Random Number Gen. (fix cabo)
			* Button in Monitor to add a specific EMM PID
	Bugfix:		* 'Can't find NagraKey.txt file' fixed
			* Scan of CAT now follows Active state

V 1.0.4.1  03-sep-22
	Minor features: * correction for Cabo
			* corrections for MyTheatre (still some bugs)

V 1.0.5.1  03-oct-07
	Minor features: * Nagra: some corrections (OTP area support mainly for Dig+)
			* Via: "Hash failed" log message for SA added

More info on http://yankse.dvbnetwork.com
and http://forums.dvbnetwork.com

----------------------
Vlinders, 2003.	