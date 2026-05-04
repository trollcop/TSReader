  CINSTALL V1.1
  Device install/remove utility for Windows 95/98

  [THE PURPOSE]
  This is a utility that allows for batch install and removal of
  Win95/98 system and network devices.  A common issue for Installer
  writers and developers is how to fully automate the installation of a
  device/network driver.  The program installs inf files by running the
  associated class installer.

  [INSTALLATION]
  The program is for Windows 95, Window 98

  No installation required. Just run "cinstall.exe" or "cinstall.exe REMOVE" (see below)

  [USAGE]
  Installation and removal of devices is controlled via the command line...

  [USAGE-INSTALL]
  To install a device you pass the class, manufacturer and description
  of the device you are going to install as command line arguments 

  e.g. cinstall.exe -c"NetClient" -m"Novell" -d"Novell NetWare Client"
  
  If the inf containing the driver details is not in the standard location
  (c:\windows\inf) then the parameter -i allows you point cinstall to an
  alternative driver location, much like the 'have disk' option within the
  'add new hardware' wizard.

  e.g. cinstall.exe -c"NetClient" -m"Novell" -d"Novell NetWare Client" -i"a:\"

  When installing the driver the driver files will be loaded from either
  the same directory as the inf or the default source directory. You
  can change the default source directory by editing the following 
  registry key,

     REGEDIT4
     [HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Setup]
     "SourcePath"="C:\\MYDRIVERS"

  [USAGE-REMOVE]
 
  To remove a device you pass the class and description
  of the device you wish to remove as command line arguments
  and append the the word REMOVE. 

  e.g. cinstall.exe -c"NetClient" -d"Novell NetWare Client" REMOVE
  
  [HINT/TIPS]

  The class of a device can be found under the [version] section of the associated
  inf file.

  To save typing cinstall provides a GUI interface to assist in identifying the
  correct command line when any of the required parameters are missing.

  e.g. cinstall -c"MEDIA" REMOVE with prompt cinstall to display a dialog listing
       all the currently installed devices of class MEDIA.			 

  When running cinstall from a batch file be sure to execute it with start.exe /w.
  If you want to remove all devices of a certain class you can use -d"*" 

  [THE STATUS OF CINSTALL]

  CINSTALL is FREEWARE.

  [COPYRIGHT INFORMATION]
  This software is provided as-is, without warranty of ANY KIND, either expressed or implied,
  including but not limited to the implied warranties of merchant ability and/or fitness for 
  a particular purpose. The author shall NOT be held liable for ANY damage to you, your
  computer, or to anyone or anything else, that may result from its use, or misuse.

  All trademarks and other registered names contained in the CINSTALL
  package are the property of their respective owners.  
 
  [BUG REPORTS]
  If you find any bugs please email them to:
  darrenvoisey@hotmail.com

  Use of this software indicates that you agree to the above conditions.

  [DISTRIBUTION]
  Program is FREE to distribute on any kind of media.
  However, if you contact me (  darrenvoisey@hotmail.com ) before putting CINSTALL
  on a CD or your website I can assure you that you are including the
  latest version of CINSTALL You will also have to mention that the people
  who buy the CD, pay for the CD and not for CINSTALL.

  Though I cannot oblige you to do so, if you put CINSTALL on a CD-ROM, I 
  would really like to receive a free copy of that CD.

  [HOW IT WORKS]
  All the APIs used are fully documented within the Microsoft Win95 DDK (look up ClassInstaller).

  [CONTACT]
  Darren Voisey
  Homepage: none
  Email: darrenvoisey@hotmail.com
  
  October 1999
  London, UK
  