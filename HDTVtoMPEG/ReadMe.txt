HDTVtoMPEG2
Copyright (C) Ben Cooley 2002
Version 1.07

----------------------------------------------------------------------------

License:

This program is free for any and all public use and modification as long as
the original creator of the program (myself) is credited.

----------------------------------------------------------------------------

This program converts HDTV MPEG2 tansport stream files to standard MPEG2 video
files playable (mostly) by media player, many software DVD player programs, and
readable by video converter programs like FlaskMPEG and Vidomi.  

ATSC MPEG2 transport stream files are the raw data sent to your Digital television
from the broadcaster, and contain multiple mpeg video and audio streams which can
be tuned by your digital tuner.  These streams differ from standard MPEG2 files in
that they encapsulate the standard MPEG2 stream data in small 188 byte packets that
are easy to transmit and decode.

HDTVtoMPEG2 simply processes the .TS transport stream files, extracting the data
from each of these 188 byte packets, and reconstructing a standard MPEG2 file from
them.

----------------------------------------------------------------------------

INSTRUCTIONS

1. Record a program from your HiPix digital video capture card or other card.  
   Some digital HDTV capture cards encrypt the data they save to the hard
   drive.  If you find this to be the case, return the card and purchase a
   card that does not encrypt free over the air digital programming (send a
   message to the manufacturer!)

2. Run the HDTVtoMPEG2 program.

3. Press the "Add" button, and find the folder where your video files are stored.
   The file dialog will automatically show files with a .TS or a .0000 or higher
   extension.  When you find your saved program, select all the files you wish to
   convert for that program by using the LEFT-MOUSEBUTTON then SHIFT-LEFT MOUSEBUTTON
   to select a range of files, or CTRL-LEFT MOUSEBUTTON to select multiple files one
   at a time.  Once you have the files you wish to convert selected, click the
   "Open" button.  All the files you have selected will be added to the "Input Files"
   list.

4. The "Output File" name will automatically be set to be the same name and directory
   as the input files, but with a ".mpg" extension and a "0000" for multiple file
   output.  You may change the name of this file, and even change the ".mpg" to ".vob"
   (the extension of DVD video files) if you would like.  However, you must have a
   digit pattern "0000" in the output file name if you want to convert input files into
   multiple output files.

5. Select the channel, or the video and audio stream id's to convert.  If you select 
   an ATSC channel, this will automatically set the Audio and video PID's, and you
   can then just convert the file.  
   
   If there is no channel information in the stream (i.e. HBO or satellite stream, you 
   must set the PID numbers for the stream manually.  For most digital HDTV broadcasts,
   this is usually 0x11 for video, and 0x14 for audio.   Occasionally it will be
   0x21 or 0x24.  Generally the first digit of the stream is the program ID in your 
   digital program guide, and the second digit is 1 for video or 4 for audio.  A future
   version of HDTVtoMPEG2 will scan your files automatically and show you the streams
   available in them, but for right now, just do a small test to make sure you have the
   right ones before starting a long conversion.

6. The "MaxFileSize" number indicates how big each of the output files will be.  A value
   of 1024 means each output file will be 1GB.  If you are writing files to a DVD-R, it
   may be a good idea to set the output file size to 4600.

7. Press "Convert" to start the conversion.  A progress dialog will indicate the current
   input and output files, and the progress on the current file.  The progress bar will
   go from 0 to 100% for each input file until all files are completed.  To interrupt
   the conversion, you can press the "Cancel" button.  If the current conversion is cancelled
   the current output files will still contain all of the valid video up to the point the
   conversion was cancelled.

----------------------------------------------------------------------------

 Changelog:

 1.0 - 8-1-2002 -		First version

 1.01 - 8-1-2002 -	Fixed a problem with some TS streams (ABC) not having the RANDOM bit in
				the adaption header set.  Now just looks for the 'start' bit.  I think
				this will get A-B frames at the beginning of a video, but it seems to 
				work.

 1.02 - 13-1-2002 -   	Fixed problem again with some TS streams not having an adaption header on
                      	TS packets marked 'START'.  This was causing some of the streams not to 
                      	convert correctly.

 1.03 - 16-1-2002 -	Fixed problem with AC3 audio stream from HBO streams.  AC3 packets from HBO
				aren't aligned to the MPEG 0xBD packets.  Bit of code realigns them.

 1.04 - 19-1-2002 -	FINALLY fixed problem with AC3 audio stream from HBO streams.  AC3 packets from HBO
				aren't aligned to the MPEG 0xBD packets, and they don't start in TS start packets 
				occasionally.  Solution is to just skip packet starts where there is no AC3 packet
				sync.  Seems to work pretty well.

				Fixed pack PTS timestamps so that the time displays in DVD players are accurate.
				One minute is now really one minute.  There's still a problem with some DVD players
				picking up the PTS in the video frames, but we'll save that for later.

				Integrated this file with both the HDTVtoMPEG2 projects and the bcdmux projects. 
				That way changes to this file automatically are reflected in both projects.

 1.05 - 2-18-2002 -	Added channel detection, support for outputting files greater than 2GB, and better
				error correction.  Also changed the program to start a new file after a selectable
				size has been reached.

 1.06 - 2-19-2002 -	Forced all files to be cut on I-Frame boundries.  Now files start on I-Frame's with
				GOP (group of picture) packets.

 1.07 - 2-23-2002 - GR: Recognise AccessDTV's strange sync byte mangling (0x47, 0x72, 0x29).
				Packet alignment per file support (.adtv files not ts packet aligned).
				Support for progress bar of total conversion.
				Improved I-frame detection, detect if picture packets are I-frames in case GOPs are missing.


