V-Plug (SoftCam Emulator for DVB-S Cards / MDAPI (MultiDec API) and DVBCore Plug-In)
------
The plug-in decrypts the Crypted words from the incoming ECM stream. The actual descrambling of the video stream is done by a special chip on the DVB card (CSA). 
This chip is available with "premium" technotrend chipset cards like Nexus. Budget cards like the WinTV Nova lack the chip. In this case you have to use software decrypt (CSA.dll) which requires a decent CPU.(-> http://csa.irde.to/) And it is a built-in feature of many DVB apps. ;) For example ALT-DVB uses windows version of FFdeCSA. By using this version, your system's CPU usage will be very low comparing to normal CSA.dll.



INSTALL
-------
Decompress the ZIP file in the root (or plugins) folder of your TV application, in example "C:\ProgDVB\". 
The installed files should look like:
C:\ProgDVB\Plugins\vPlug.dll
C:\ProgDVB\Plugins\vplug.ini
C:\ProgDVB\Plugins\Softcam.key
C:\ProgDVB\Plugins\au.Bin
C:\ProgDVB\Plugins\v_sid_cache.xml
C:\ProgDVB\Plugins\rsakeylist
C:\ProgDVB\Plugins\v_biss.ini
C:\ProgDVB\Plugins\v_dcw.ini
C:\ProgDVB\Plugins\v_verify.ini
C:\ProgDVB\Plugins\v_nagra2_emm.ini
C:\ProgDVB\Plugins\v_emm.ini
C:\ProgDVB\Plugins\v_get_key.ini
C:\ProgDVB\Plugins\v_Radegast.ini
C:\ProgDVB\Plugins\v_dcw_sharing.ini
C:\ProgDVB\Plugins\v_AES.db
C:\ProgDVB\Plugins\Noone.srv
C:\ProgDVB\Plugins\image.bin


V-Plug has been tested with:
	MD-API based programs:
		- ProgDVB (installation dir = Root or Plugins folder of the app.)
		  			http://www.progdvb.com/
		- ALT-DVB v1.0.7+ (installation dir = Plugins folder of the app.)
		  			http://www.altdvb.ro/
		- HispaDVB v2.0+ (installation dir = Plugins folder of the app. + CSA.DLL for Budget cards in the modules folder)
	        	http://www.hispadvb.com	  
	  - TSReader (installation dir = MDPlugins folder of the app. + CSA.DLL in the root folder)
	          http://www.coolstf.com/tsreader/      				          
	  - DVB Dream v1.2 (installation dir = Plugins\pip00 ,...)
	          http://www.dvbdream.org/      
	          

	DVB-Core based programs:
		- MyTheatre v3.x (installation dir = Plugins folder of the app.)
					  http://www.dvbcore.com/
		- RitzDVB v0.7.1.1 (installation dir = Plugins folder of the app.)
					  http://www.ritzdvb.com/	  
        	  

  And also:
    - WatchTVProEx v1.41 (installation dir = Plugins folder of the app + set use_TTTrick=1 in vplug.ini file. Also you need a patched firmware, eg. from ProgDVB ! (Copy contents of C:\ProgDVB\Modules\boot\24  to C:\WatchTVPro\Modules\TTPremium\Firmware\Boot\24).) 
            http://www.watchtvpro-experience.de




History
-------
16.10.2006
V 1.3.4
-Bug fix  : Fixed DISH & BEV Autoroll. 





13.10.2006
V 1.3.3

-Improved : Added N2 EMM Nano B1 processing for DISH and BEV (DISH & BEV Autoroll). 
-Bug fix  : Processing Windows shutdown message.  
-Improved : Added BISS & constant dcw's support for WatchTVPro.






04.10.2006
V 1.3.2
-Bug fix : Fixed some issues with MT(v3.36). (Thnx J.F.)






29.09.2006
V 1.3.1

-Update : Some internal fixes, including:
						-Delete Noone.dll and use new Noone.srv file. Now dvb-app's won't try to load it as a plugin.
						Or
						You can move all of the vPlug's related files (except vPlug.ini+vPlug.dll) to for example c:\xyz1234
						Then just specify it in vPlug.ini file ->FilesPath.
						-Fixed freezes with MT.






27.09.2006
V 1.3
-Improved : Added New NA N2 MECM$40 handling. (Thnx Noone!)
-Improved : Added TTTrick support for the SS1-TT KS driver. Set use_KS_driver & use_TTTrick to 1 in vPlug.ini. (Thnx to Nagravision for the info. and testing)
-Improved : Added AutoPMT for the apps which do not parse PMT correctly (for example you see 2 ecm pid's on the specific channel but it has only one ecm pid). Set AutoPMT to 1 in vPlug.ini in these cases.
-Bug fix : Automatically starting the cs client when set "AutoStartCardSharing=1" in vPlug.ini. 
-Improved : Detection of N2 EMM's in different dvb apps.
-Improved : Get "op-key" from URL. Now v_get_key.ini supports all of the public systems which are implemented in vPlug. (Set InternetAccess to 1)
		Its new format:
			[url-desc]
			site_url=http://xxx
			regular_exp_pattern_to_find_key=yyy
			key_part1_exp=optional
			key_part2_exp=optional
			key_index=idx
			Provider_id=prvi
			System=s






07.09.2006
V 1.2.1

-Bug fix : Fixed some issues with daylight saving time.
-Improved : AES Keys AutoCycle by using v_AES.db keys.





01.09.2006
V 1.2.0

-New: Added v_AES.db . Now you can watch TPS channels for a week *without* an internet connection.
	Notes:
	1- Please adjust your "system time" and also "time zone" in control panel. Otherwise it won't work as expected.
	2- If your time zone in windows registry is not correct, by using the tzedit program, you can edit it. 
	For example changing "offset from GMT" +3:30 to +4:30 or disabling daylight saving time.
	h++p://www.softshape.com/download/tzedit.zip
	3- If you want to disable AutoAESfromWeb, set InternetAccess to 0 in vPlug.ini file or use preferences menu.
	4- Format of v_AES.db is very simple: (if you want to make it yourself for another week)
	Plain AESkey(16 byte) + UNIX time(4 byte) + ...






26.08.2006
V 1.1.0

-Improved : Radegast-CardClient. (Thnx to Predator314, Skywise & Daex for testing)
-New : Added dcw's logger to Radegast-CardClient. (Also format of v_Radegast.ini has been updated)
			format of each line in its log file is:
			CRC32 of the ecm packet:dcw odd/even index:DCW:date time
			Ex:
				52E14418:0:8103E6F96B6E8B60:2006/08/25 01-46-00 
-Bug fix: Some issues with dvb-core based apps have been fixed.
-Bug fix: Fixed ecm decryption with differnt packet sizes.





24.08.2006
V 1.0.0

-Update : Added AutoReconnet to Radegast-CardClient.
-Update : Added proxy settings for the CardClient, AutoAESfromWeb and getTransponders.
-Bug fix: Fixed calculating hash of TPSCrypt2 when 2 "F008" are present in the ECM packet!
-Bug fix: Fixed crash on exit.





20.08.2006
V 0.9.9

-Update : Some internal fixes, including:
							- "AutoStartCardSharing" option in vplug.ini. Set it to 0 to disable starting card sharing on channel change.
							- Set "DisplayErrors" in vplug.ini file to zero to disbale system tray icon. (Or use preferences menu)
							- Some fixes for save to file option of Irdeto1.
							- Now specifying correct CA_ID and Provider_ID in v_Radegast.ini for channels with 1 ecm-pid and 1 entry in .ini file are optional. 
							- Faster channel switching with Radegast servers.





18.08.2006
V 0.9.8

-New : Radegast-CardClient. You can add your Radegast servers to v_Radegast.ini file. (Many thnx to Fedmond31 for the test server)
-Update : NA N2 AU fix. (Thnx to Daemonx for the info.)
-Update : Some internal fixes.





09.08.2006
V 0.9.7

-Update : Menu_id of "Transponders update" has been changed.
-Update : Some fixes for DCWs-Sharing through web pages. (less freezes)





05.08.2006
V 0.9.6

-Update : DreamTV Nagra1 AU.
-Update : Some fixes for AutoAESfromWeb.
-New : Transponders update from the net.
-New : DCWs-Sharing through web pages. 
       You can add your card sharing servers which can publish dcws through web pages in v_dcw_sharing.ini file.
       (It's a very simple and one way card sharing method) 
       Structure of v_dcw_sharing.ini is similar to v_get_key.ini file. (Thnx to Tonytr for the test server)
       You can track dcws by using update key tab.





15.07.2006
V 0.9.5

-Update : Freezes during the AES key changes have been fixed. (It needs a good server for sure)
-Update : Automatically disabling use_TTTrick when it is not supported.






07.07.2006
V 0.9.4

-New : Offline Debugger.
-Bug fix : Fixed broken N2 ECM handling.
-Bug fix : Adding non-repetitive new found AES key to the end of Softcam.Key file.






07.07.2006
V 0.9.3

-New : Preferences editor.
-Update : Some fixes for AutoAESfromWeb.
-Bug fix : Error in calculating hash of Viasat3 (Viaccess PC-2.3) has been fixed.
-Bug fix : ECM parsing of Cryptoworks has been fixed.






24.06.2006
V 0.9.2

-Bug fix : Fixed Crash on exit.
-Update : v_get_key.ini file has been updated. 





23.06.2006
V 0.9.1

-New : Cryptoworks emulation has been Added. (Thnx to st20tp2)
       SoftCam.key format: (similar to camd3.keys)
           ;W -> Cryptoworks ; 0DnnYY -> 0Dnn=CAID, YY=Provider id
           W 0DnnYY 00 xxxxx ; 32 characters = ECM Key index 00
           W 0DnnYY 01 xxxxx ; 32 characters = ECM Key index 01
           W 0DnnYY 06 xxxxx ; 12 characters = CardSystem Key
           W 0DnnYY 80 xxxxx ; 32 characters = EMM-G Key index 00
           W 0DnnYY 81 xxxxx ; 32 characters = EMM-G Key index 01
           W 0DnnYY B0 xxxxx ; 128 characters = RSA Key
-Update : v_get_key.ini file has been updated. (Thnx to Zakari)
-Bug Fix : Writing Via-1's found key to softcam.key file.
           A quick how-to: (Via-1 AU without internet connection)
          	a) you should have au.bin file.
          	b) tune to one of the SRG's channels.
          	c) open vPlug's monitor and select emm tab.
          	d) click at find pid's.
          	e) select emm-pid of 009400 from the dropdown list.
          	f) click at start button and wait. (max. 5 minutes)
          	or
          	c,d,e,f) -> edit v_emm.ini file like this:
          			[009400]
						    emm_pid_hex=0259
              	AutoTerminateOnKeyFind=1
              	AutoStartOnChannelChange=1
-Update : "Reread SoftCam.Key" menu option has been removed. Now its changes are being watched.




15.06.2006
V 0.9.0

-Bug Fix: fixed N2 dcw's parsing (moderate intermittent freezes). (Thnx to Daex)





13.06.2006
V 0.8.9

-Update: DigiTV (N2 2111) fixed. You need the correct keys for sure! (Thnx to omega_man for the help and testing)
-New : Added CardClient CAMD3.5 UDP protocol support (ECM request). (Thnx to J.T.)
Note: Only tested with camd3.exe for cryptoworks channels of W3.
	You can enable/disable it in vplug.ini and it is possible to specify the exe path of camd3 to let vPlug run it automatically (optional).
	Console's out put of CAMD3 for win32 can be traced by using "win camd3" tab of vPlug in this case. 
	It is not perfect but works (partially)!
-New : N2 mecm processing. (Thnx to Jake for the Algo and Thnx to Daex and Drummerboy for the help and testing)






04.06.2006
V 0.8.8

AutoAESfromWeb Updates:
  - Do not caching web pages.
  - Validating new found key before writing it to SoftCam.key file.
  - Testing all of the v_get_key.ini entries one by one until finding the working key and not only the 1st item. So please be patient!
  - Adding non-repetitive new found AES key to the end of Softcam.Key file for the next AES Keys AutoCycle test.
  Note: All of the related settings will be read from IE's preferences (such as proxy settings, etc.)

-Bug Fix: fixing broken saving Analyzed ECM's output option.






02.06.2006
V 0.8.7
-Bug Fix: Do not using illegal MD-API Filter_ID's!
-New: AES Keys AutoCycle.

  Note: You can add as many AES key as you want to softcam.key file! Index of keys are not important here and can be 01,xz, x1,... or any thing else with length of 2 characters. And plz add ';' before your comments in SoftCam.key file!

-Update: AutoAESfromWeb with correct http header info.
	
	Note: vPlug only tries to find new AES key when the hash is incorrect (every 30 minutes/until next key change). So it does not cause a high load on web servers (such as other AES key finders with key update interval set to every 1 minute or less)

-Bug Fix: fixing broken saving ECM packets option.
-New : "Reread SoftCam.Key" menu option.






25.05.2006
V 0.8.6
-Bug Fix : Hex Viewer errors.
-Bug Fix : P*e*i*re (N2 0501) fixed again!






25.05.2006
V 0.8.5
(Maintenance release)
-Update  : Nagra2 Nano E0 handling. (Thnx to Daex)
-Update  : Detection of CA System Names based on latest official docs. (http://www.dvb.org/products_registration/dvb_identifiers/ca_systems/)
-Bug Fix : Some little bugs in PMT parser have been fixed. 
-Bug Fix : Bad processing of nano83 in nagra2 EMM parser has been fixed. 
-New : FilesPath option in vplug.ini file. (If this option sets to empty, plug-in will read all of the related files from where the dll is located (default value). if you specify a path (for example a network location or a central path for all programs), new read/write path will be this option's value. (except vplug.ini file))
-New : AutoAESfromWeb option in vplug.ini file.(If your softCam.key file is not up-to-date it will try to find the new AES key from the web automatically and softCam.key file will be updated. set AutoAESfromWeb=0 to turn it off (default value).)
-New : Displaying info's/errors by using a tray icon. (Set DisplayErrors=0 in vplug.ini file to turn it off.)






30.03.2006
V 0.8.4
-New : TPS "AES" key updater from internet (new "update key" tab and v_get_key.ini file). (Thnx to Navbas for different Regular Expression patterns)
-New : Binary logger (ECM/EMM) 
-Bug Fix : D+ Nagra2 AU.
-Bug Fix : Fixed Crash on exit with TSReader. 
-Bug Fix : Rewriting found Nagra2 keys if different old keys set to the same value!
-Bug Fix : Detection of Globecast as a Nagra2 provider.





21.03.2006
V 0.8.3
-Bug Fix : POLSAT's Nagra2 AU.





16.03.2006
V 0.8.2
-Bug Fix : Small freezes with Nagra2.
-Bug Fix : Nagra2 emm handling.(Thnx to Drummerboy & Daemonx)






11.03.2006
V 0.8.1

-New : Globecast-Nagra2 support. (Plz Remove all of the old Nagra1 keys and Try with a correct RSA key...)
-New : PolSat-Nagra2 support. (Thnx to "NIST Special Publication 800-20, Figure 2. TDEA Cipher Block Chaining (TCBC) Mode, Page 8"!)
-Bug Fix : Constant DCWs detection and DVBCore apps.
			
			
			"Knowledge has a value when you share it with others..."





03.03.2006
V 0.8

-New : Reading delay of sending DWs from vplug.ini file. by playing with the setting delays for the dw's you can effectively eliminate most of the momentary freezing and pixelations.
-New : TTTrick for WatchTVPro (use_TTTrick=1 in vplug.ini file). It is not working with other programs, only TTDVBACC applications. Also you need a patched firmware, eg. from ProgDVB !  (Copy contents of C:\ProgDVB\Modules\boot\24  to C:\WatchTVPro\Modules\TTPremium\Firmware\Boot\24).
-Bug Fix : Nagra2 emm handling.(Thnx to Daemonx)





11.02.2006
V 0.7.9

-Update : Nagra2 EMM handling and bad EMM-G packets. (Thnx to Daemonx for all the help and testing)
-Update : Testing Nagra2 ECM decryption when verify key is unknown!
					Ex. For TvCabo add the following line to v_verify.ini file:
					[nagra2]
					4901 = 11111111111111111111111111111111
					In this way (11...<--16 times-->...11) plug-in will not check the hash. (thnx to wlx for testing) 





09.02.2006
V 0.7.8

-Bug fix: Stopping CAT filter.





08.02.2006
V 0.7.7

-New: Calculating hash of Via-1/2 and TPSCrypt1/2.
-New: Calculating hash of Seca-1.
-New: Auto ECM. (Auto Cycle between different ECMs of one provider and selecting the decryptable one based on correct hash of decoding.)
-New: v_sid_cache.xml file. (its records will be generated by plug-in based on successful pids of AutoECM)
-Bug fix: Crash on_exit has been fixed (BMDThread was removed!).
-Bug fix: Nagra2 EMM handling and fake keys. (Thnx to Daex for the info. and testing)
-Update: tvCabo -> Nagra2 ECM handling was fixed. (Remove Nagra1 keys and then test it + add its verify key to v_verify.ini file)      





08.01.2006
V 0.7.6

-Update: D+ Nagra2 ecm decryption was fixed. (thnx to Cookie and Testi for the info.)
-Update: Using normal Nagra2 key format
	  				 N 0101 00 xxxxxxxxxx... ; (86)->key00
		  			 N 0101 01 xxxxxxxxxx... ; (96)->key10 or key01 here
			  		 N 0101 M1 xxxxxxxxxxxxxxxxxxxx...
-New: Menu has been added to main form to facilitate start/stop of the plug-in in dvbCore conditions.
-Update: CPU meter was removed. (one of the reasons of instability)
-Update: Better code clean up at On_Exit. (thnx to JoshyFun and his great PoorMan'sCam's sources, http://joshyfun.peque.org/)
-Update: Main dialog form of the plug-in is not always on top any more.
-Bug fix: No more Unknown CA Systems!
-Bug fix: Stopping AutoEMM filters.
-Update: AutoStart nagra2 emm filter on incorrect hash of ecm decoding (it reads correct emm pid from v_emm.ini file).
-Update: It seems sleep(100) is necessary between sending dcw1 and dcw2 for dvbCore based apps.




30.12.2005
V 0.7.5

- New : DVBCore support! 
				(Never update the UI on a thread other than the UI was created upon! ;))
- update : YANKSE-sid.cache file has been updated. (thnx to its author, LeDoc - h++p://membres.lycos.fr/ledoc1/)
- update : Threading model has been changed. (thnx to BMDThread components and its author, Boian Mitov - w*w.mitov.c*m)
- bug fix : locking of the find emm pid's button.
- New : Auto Start/Stop of EMM filter by using v_emm.ini file.




23.12.2005
V 0.7.4

- update : BEV Nagra2 ECM and EMM handling.
- update : YANKSE-sid.cache file has been updated for DISH & BEV channels. (thnx to Smilex745)
- update : v_nagra2_emm.ini file has been updated for BEV channels. (thnx to Drummerboy and also for his great testing report of the plug-in)




16.12.2005
V 0.7.3

- new: Nagra2 EMM handling. (Thnx to CardCoders & Jake)
		How to use:
			Select the EMM page.....find PIDs, select xxxx pid, and start. 

- update: support for Nagra2 ECM IDEA keys/opkeys (86)->key00 and (96)->key10.
Key Format: 
		N 0101 00 xxxxxxxxxx... 
		N 0101 10 xxxxxxxxxx...
		N 0101 M1 xxxxxxxxxxxxxxxxxxxx...




18.11.2005
V 0.7.2

- Update: Long ECMs Handling of Nagra2. (Thnx to CardCoders)




10.11.2005
V 0.7.1

- New: Reading verify keys of nagra2 and nagra1 from v_verify.ini file.





25.10.2005
V 0.7

- NEW: Nagra2 ECM decryption. (Thnx Humax, siko-SK, st20tp2, Remote ...).
       softcam.key format:
         N 7101 I1 xxx... ; idea key ('I1' instead of '00' to avoid conflict between nagra1's key00 and nagra2's idea key)
         N 7101 M1 xxx... ; ecm mod
- NEW: Nagra-1 PK0 finder. (Thnx RichardF for the info.)
- NEW: ECM-Check tab.
- NEW: PAT filter.
- NEW: "NT" based CPU meter.
- Bug fix: checking expired sid.cache ecm-pids.
- NEW: fixed DCW manager (v_dcw.ini file).




13.05.2005
V 0.6

- Bug fix: Bug in ECM decryption algo of Nagra, when they do not use RSA algo.
- Improved: Detecting of more useful Nagra emm's for AU.
- NEW: Irdeto-1 ECM decryption. (Thnx F.B. for the ECM Log)
- Improved: Constant DCW Tab.
- Bug fix: Detecting BISS system.
- NEW: Displaying calculated check sums of Biss key (useful for normal receivers with BISS option).
- NEW: Reading BISS keys from v_biss.ini file. (Plz refer to its format for more info)

PS. BISS encryption system just uses "fixed DCWs". There is no algo...




22.04.2005
V 0.5

-NEW: Via-2 ECM decryption. (And also YANKSE-sid.cache has been updated for ART channels)
(A lot of thnx to the author of "Russian wafer" card & MrToolate)
-NEW: Nagra-1 EMM decryption ( Just Polsat AU! ... I can not receive other Nagra providers :( ).
-NEW: Writing found Nagra's keys to SoftCam.Key file.
-NEW: Writing found Via-1's keys to SoftCam.Key file.
-NEW: Reading RSA Keys of Nagra EMM decryption from "rsakeylist" file.
-NEW: Integrating ECM calculator and plug-in. (my little app to test algo's)
-Improved: Detecting of CA System Names. (Many thnx to the author of dvbsnoop, http://dvbsnoop.sourceforge.net/)




26.03.2005
V 0.4

-NEW: Nagra-1 ecm decryption (Just Polsat! And you should have its RSA keys in SoftCam.Key).
-NEW: Reading "YANKSE-sid.cache" file. (To find correct ECM-PID -> Faster channel switching!)
-NEW: Reading related files (softcam.key, au.bin, .ini & YANKSE-sid.cache) from where the dll is located. 
-NEW: CAT filter. (Thnx JF for the "How to write a SECA CAM " doc.)
-NEW: Creating filters over the any arbitrary PID (For debugging purposes). 
-NEW: Reading "TPSCrypt2" keys from softcam.key (Ex.: V 007C00 01 AES_key (16 bytes=32 HEX digits)). AES hard coded key has been removed.
-NEW: Reading Via-1 SA/MK from au.bin file. (format of this file is according to sammy/hummy AU ram.bin files!)
-Bug fix : EMM filter is more stable now! 
-Bug fix : Controlled "CPU index out of bounds" errors. ;)
And many thnx to the authors of "Advanced Encryption Standard (AES), Delphi implementation (http://www.eldos.org)" and "CryptLIB (http://www.cri.ch/sven/technical/dev/CryptLIB/default.html)" for fast RSA routines.




xx.xx.2005
V 0.3 a/b

- New TPSCrypt2's AES keys -> patched by rofaghaa! (never released by me!)




18.02.2005
V 0.3

-NEW: TPS AU Fix! (Thnx MrToolate!)
-NEW: Seca-1.  (Many thnx to the authors of "MEDIAGUARD MUSINGS" and "GrolandSAT Plug-in"!)
-NEW: PMT filter. (Thnx Remote!)
-NEW: Scan_Current_CAT -> Enhanced emm filter!  (Thnx Vlinders for the CAPI_Sample!)
-NEW: Get_MDAPI_Version
-NEW: Activate/deactivate option is working now!
-NEW: CPU Usage Measurement. (Thnx Dynnikov->http://www.aldyn.ru/)
-Bug fix: starting/stopping ecm/emm filters on selected pid.
-Bug fix: TPSCrypt1 detection/TPSCrypt2 odd key! (Many thnx to France 2/3/5 channels!)




24.12.2004
V 0.2

-Constant DCW support ;)
-Bug fix: Via-1 odd key
-BISS Support (BISS CAID =0x2600, thnx Zhanmo1!)
-Bug fix: Duplicate Get_Programm_Detail has been removed.
-Bug fix: Setting Form_Close Action to caHide.
-Low CPU usage! (Sending DCWs when it is necessary)




17.12.2004
V 0.1

First public release!
Tested in ProgDVB.
Supported systems:
-Via-1
-TPSCrypt-1
-TPSCrypt-2.1
And Via-1 EMM analyzer (Beta Ver.!!)

Many thanks to MrToolate, VKeys & DVBN Friends!



======================

Disclaimer-1:
If it's working, great, if not...then just delete it (I recommend "shift+delete"...). 

Disclaimer-2:
This tool is strictly for Educational purpose only. The author is not to be made liable for abuses. 

======================

Greetings from Iran...