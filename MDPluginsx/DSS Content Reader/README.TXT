    __________________________
    |                         |
    | DSS Content Reader 1.0  |
    |    Beta 3 (Preview 2)   |
    |    August 23rd, 2005    |
    |_________________________|

    ____________________________________________________________________

    Setup Instructions:

    1) Install TSReader (or TSReaderLite) 2.6.41 or later 
        (http://www.coolstf.com/tsreader)

    2) Install the Java Runtime Environment version 1.5 or later
	(http://www.java.com)

    3) Install VLC Media Player 0.8.1 or later (For Video Support)
        (http://www.videolan.org)

    4) Open TSReader, tune it to a DSS source, and close it

    5) Copy "DSSCR_TS.DLL" to TSReader's "MDPlugins" subdirectory

    6) Copy "DSSCR.jar" and "dsscr_ts.ini" to a directory of your choice

    ____________________________________________________________________


    Quick Usage Instructions:

    1) If TSReader is open, it MUST be closed

    2) Double-click on "DSSCR.jar" to launch DSS Content Reader

    3) Press "Tune" in the TSReader window that pops up

    4) Double-click on channels in the "Guide (STD)" panel to change
        channels, or use the radio buttons in the "Music" panel to
        switch between music channels on the current transponder

    5) When you're done, close the 'DSS Content Reader' window.
        This will automatically close TSReader and VLC if they are open.

    ____________________________________________________________________


    Additional Instructions:

    - To prevent VLC from freezing up its video playback (for no good
        reason that I can discern), in VLC go to Settings -> Preferences
        and then to Input / Codecs -> Access Modules -> HTTP/HTTPS.
	Check the "Advanced Options" box and change the "Caching value
        in ms" field to a value of 8000-10000.
        Click the "Save" button to save the updated VLC configuration

    - To improve the video quality and display, in VLC go to Settings ->
        Preferences and then to Video. Check the "Advanced Options" box
	and change the "Video width" to 640 and the "Video height" to
        480. 
        Next, go to Video -> Filters and check the "Deinterlacing Video
        Filter" box. 
        Finally, go to Video -> Filters -> Deinterlace. Here, set the
        "Deinterlace mode" fields to Blend. 
	Click the "Save" button to save the updated VLC configuration

    ____________________________________________________________________


    Known Issues and Bugs:

    - When closing the program, if TSReader was open it will display
        a memory error. Just click OK and it will go away quietly.

    - Sometimes audio doesn't play automatically on startup or when
        changing channels. This problem should be fixed soon.

    - Sometimes audio is lost during playback and doesn't return.
        This problem should be fixed soon.

    - The program prints lots of debug messages to the standard 
        output. These will be removed when the program is more stable.

    - The guide display may not be correct when month transitions 
        take place since it hasn't been tested with them

    - Some extraction buttons may be enabled when they shouldn't be,
        and I don't feel like fixing that right now :-)
