                              DISKUSE Version 1.5
                            PM Graphical Disk Usage
                                      by
                                 Ric Zapanta


DESCRIPTION:

DISKUSE is a 32-bit, multi-threaded OS/2 Presentation Manager (PM) application
written in C which displays your drive usage in a graphical manner (color pie
chart with optionally displayed percentages).

Three other PM windows are displayed together with the main window:

 o  Directory List

    The complete directory tree including directory and cumulative sizes
    and percentages as well as the number of files.

 o  Legend

    Legend for color pie chart displayed in primary window.

 o  All Drives Information

    Display usage, free and total size, for all drives found in the
    system along with a graphical representation (like a fuel gauge).

To run, just type DISKUSE <drive> <parms> from an OS/2 command prompt
or create a program object on the WPS (the icon is built-in to the
program). If drive is not specified, the current drive is used. You may change
drives in the Select Drives choice under the Disk menu.

Optional command line parameters are:

  /P  specifies the location of the profile (DISKUSE.INI), use
      /P=D:\PATH or /P- to use the system defaults instead of the profile
  /D  /D+ or /D- displays or supresses the directory list irrelevant
      of the setting in the profile
  /L  /L+ or /L- displays or supresses the legend irrelevant of the
      setting in the profile
  /A  /A+ or /A- displays or supresses the all drive information
      irrelevant of the setting in the profile