:userdoc.
:h1 res=9110.PM Graphical Disk Usage
:i1.General Description
:p.DISKUSE is a 32-bit, multi-threaded OS/2 Presentation Manager (PM)
application which displays your drive usage in a graphical
manner (color pie chart with optionally displayed percentages).
If you double click on a pie slice, the details of that directory is
displayed in a dialog box.
Three other windows are displayed together
with the main window:
:ul.
:li.Directory List
:p.The complete directory tree including directory and cumulative size
and percentages as well as the number of files. If you click on the right
mouse button, you may move, delete, copy or make virtual root the selected
directory.
:li.Legend
:p.Legend for color pie chart displayed in primary window.
:li.All Drives Information (Summary)
:p.Free space and total size for each valid drive attached to your system
is listed along with a small graphical representation (similar to a fuel
gauge) to indicate the drive's usage.
:eul.
:p.From the Options menu bar choice, you may either select to customize your
settings (such as graph, units, initial views, etc.)
refresh the drive information. From the Drives menu bar choice, you can
select a new drive.
From the View menu bar choice, you may display or remove (toggle)
the Directory List window, the Legend window and the All Drives Information
window.
:p.There is a limit of 50 for the maximum number of pies because smaller
pie slices become difficult to display. If you have more subdirectories than
the maximum number of pies, the smaller directories are all placed together
in Other.
:p.Just type DISKUSE <drive> <parameters...> from any OS/2 command prompt.
If no drive is specified, the current drive is used.
For example, DISKUSE D: will use the D: drive.
:p.The optional command line parameters are:
:ul.
:li./P to specify the location of the DISKUSE profile file
(use /P=D&colon.\PATH) or to use the system default settings (use /P-).
:li./L to specify if the legend should be shown initially or not (use /L+
or /L-) irrevelant of the profile.
:li./D to specify if the directory list should be shown initially or not
(use /D+ or /D-) irrevelant of the profile.
:li./A to specify if the all drives information window
should be shown initially or not (use /A+ or /A-) irrevelant of the profile.
:eul.

:h1 res=9510.Help for Directory List Window
:i1 id=Dirlist.Directory List Window
:p.The Directory List window lists the directory tree for the default drive.
:p.The size for each subdirectory along with the corresponding percentage is
given. The subdirectory cumulative size, which is the total size of the
subdirectory and all its subdirectories, along with the corresponding
percentage is also given.
:p.Finally, the number of files for each
subdirectory is listed and is not cumulative.
:p.You may show or hide this window by clicking on the Directory List
option under View.
:p.To select a directory, click on the left mouse button. Then click on the
right mouse button to bring up the Directory List pop-up menu. You may
select the following actions:
:ul.
:li.Move directory
:li.Delete directory
:li.Copy directory
:li.Make the directory the root directory
:li.Go to the real disk root directory
:eul.

:h1 res=9520.Help for Directory Size
:i2 refid=Dirlist.Directory Size
:p.There are two directory sizes shown in the Directory List window:
:ol.
:li.Dir. Size is the size of the subdirectory. This is used when
the All directory level option is selected.
:li.Cum. Size is the cumulative size of the subdirectory which is
the directory size plus the directory sizes of all subdirectories under it.
This is used when the 1st level directories only option is selected.
:eol.

:h1 res=9530.Help for Legend Window
:i1 id=Legend.Legend Window
:p.The subdirectory name corresponding to each graph pie is shown in the
scrollable Legend window.
:p.You may show or hide this window by clicking on the Legend
option under View.

:h1 res=9540.Help for All Drives Information Window
:i1 id=All.All Drives Information Window
:p.The free space and total size of each drive attached to the system is
shown, along with a graphical respresentation (fuel gauge) of the drive's
usage, in the All Drives Information window.
:p.You may show or hide this window by clicking on the All Drives Info
option under View.

:h1 res=9120.Help for Options
:i1 id=OptMenu.Options Menu
:p.Use this choice to customize several settings,
(such as graph, units, initial views, etc.) or to refresh (re-read)
the drive information (in case there have been changes).

:h1 res=9124.Help for Settings
:i2 refid=OptMenu.Settings...
:p.Use this choice to customize:
:ul.
:li.Options
:ul compact.
:li.Directory list displayed initially or not
:li.Legend displayed initially or not
:li.All Drives Information displayed initially or not
:li.Confirmation upon exit
:li.Refresh confirmation prompt
:li.Units (bytes, kilobytes or megabytes)
:eul.
:li.Customize graph
:ul compact.
:li.Graph type (use total space or ignore free space)
:li.Graph level (show all subdirectories or only first level subdirectories)
:li.Graph percentages or actual sizes (show or hide)
:li.Display SWAPPER.DAT information
:li.Maximum number of pies in chart (between 1 and 50)
:eul.
:li.Automated refresh settings
:ul compact.
:li.Automated refresh interval time
:li.Threshold values (in %) for free space and SWAPPER.DAT sizes
:li.Warning of notification types when threshold limits passed
:eul.
:eul.

:h1 res=9122.Help for Refresh Now
:i2 refid=OptMenu.Refresh Now
:p.Use this choice to refresh or
re-read the drive information to pick up the latest changes.

:h1 res=9134.Help for Fonts
:i2 refid=OptMenu.Fonts...
:p.Use this choice to select a new font.

:h1 res=9136.Help for Print
:i2 refid=OptMenu.Print...
:p.Use this choice to print the directory list and optionally print the
pie chart.

:h1 res=9126.Help for Drives
:i1 id=Drive.Drives Menu
:p.Use this choice to select which drive you wish to display the disk usage.

:h1 res=9130.Help for View
:i1 id=View.View Menu
:p.Use this choice to toggle the display of the Directory List, Legend and
All Drives Information windows.

:h1 res=9132.Help for Directory List
:i2 refid=View.Directory List
:p.Use this choice to toggle the display of the Directory List window.

:h1 res=9131.Help for Legend
:i2 refid=View.Legend
:p.Use this choice to toggle the display of the Legend window.

:h1 res=9133.Help for All Drives Info
:i2 refid=View.All Drives Info
:p.Use this choice to toggle the display of the All Drives Information
window.

:h1 res=9210.Customize Graph Help
:i1 id=Options.Customize Graph
:p.You may customize several settings from Graph:
:ul.
:li.graph type (use total space or ignore free space)
:li.graph level (show all subdirectories or only first level subdirectories)
:li.graph percentages or actual sizes (show or hide it)
:li.display SWAPPER.DAT information
:li.maximum number of pies in chart
:eul.
:p.After changing any settings, close the window. Select Default to
restore the settings to the original system defaults.

:h1 res=9213.Graph Type Help
:i2 refid=Options.Graph Type
:p.There are two available graph types:
:ul.
:li.Total space (free or unused space included)
:li.Used space only (exclude free or unused space)
:eul.

:h1 res=9214.Graph Level Help
:i2 refid=Options.Graph Level
:p.There are two available graph levels:
:ul.
:li.All directories (do not cumulate subdirectory sizes)
:li.1st level subdirectories only (use cumulative directory size)
:eul.

:h1 res=9217.Graph Display Help
:i2 refid=Options.Graph Display
:p.You may display the directory percentages or the actual directory sizes
on the pie chart. Those pie slices under 2% are not shown regardless of
this setting.
:p.If you do not want to display this detail on the pie chart, click on None.

:h1 res=9215.Graph Maximum Pies Help
:i2 refid=Options.Graph Maximum Pies
:p.You may type a number between 1 and 50 for the maximum number of pies to be
displayed in the pie chart. Please note the following:
:ul.
:li.If you have more subdirectories than the specified maximum number of pies,
the smaller subdirectories are placed in Other.
:li.The maximum number of pies includes Other and Free space (if applicable).
:li.A limit of 50 for the maximum number of pies has been chosen because
the smaller pie slices will be too small to display.
:eul.

:h1 res=9216.Default Help
:i2 refid=Options.Graph Default
:p.Select this push button to restore the Options settings to the
original system default values.

:h1 res=9219.SWAPPER.DAT Information
:i2 refid=Options.SWAPPER.DAT Information
:p.You may display the size of the SWAPPER.DAT file as a separate pie slice
on the graph. As well, you can also display the size of the SWAPPER.DAT
on the bottom status area on the main window. Click on the appropriate
check boxes to activate or deactivate either option.
:note.The SWAPPER.DAT information will only be displayed if the current drive
contains the SWAPPER.DAT file.

:h1 res=9410.Options Help
:i1 id=Settings.Options
:p.You may customize several settings from Options settings:
:ul.
:li.Directory list displayed initially or not
:li.Legend displayed initially or not
:li.All Drives Information displayed initially or not
:li.Confirmation upon exit
:li.Refresh confirmation prompt
:li.Select units
:eul.
:p.After changing the options, close the window. Select Default to
restore the settings to the original system defaults.

:h1 res=9411.Directory List Settings Help
:i2 refid=Settings.Directory List settings
:p.Click on to display the directory list window initially.

:h1 res=9412.Legend Settings Help
:i2 refid=Settings.Legend settings
:p.Click on to display the legend window initially.

:h1 res=9418.All Drives Info Settings Help
:i2 refid=Settings.All Drives Info settings
:p.Click on to display the all drives information window initially.

:h1 res=9413.Exit Settings Help
:i2 refid=Settings.Exit settings
:p.Click on to display an Exit confirmation dialog box when you exit DISKUSE.

:h1 res=9415.Refresh Settings Help
:i2 refid=Settings.Refresh settings
:p.Click on to display an Refresh confirmation dialog box when you select
a new drive.

:h1 res=9416.Units Help
:i2 refid=Settings.Units
:p.You may select which units are to be used in the application. The
following units are available:
:ul.
:li.Bytes
:li.Kilobytes (KB) which is Bytes / 1024.
:li.Megabytes (MB) which is Kilobytes / 1024.
:eul.

:h1 res=9414.Settings Default Help
:i2 refid=Settings.Settings Default
:p.Select this push button to restore the Settings to the
original system default values.

:h1 res=9417.Directory Tree Help
:i2 refid=Settings.Directory Tree
:p.Select where the directory tree is to be shown on the Directory List
window.

:h1 res=9310.Refresh Dialog Help
:i1 id=Refresh.Refresh Dialog
:p.Select OK to continue. The information on the drive will be re-read so
this may take a while (depending on the size of the selected drive). Select
Cancel if you do not wish to continue.

:h1 res=9211.Drives Help
:i2 refid=Drives.Drives
:p.You may select which drive to use. Only the available drives are shown
in the combination list box.

:h1 res=9710.Automated Refresh Help
:i1 id=Autoref.Automated Refresh
:p.You may customize several settings from Automated Refresh:
:ul.
:li.Refresh interval (you can specify the minutes)
:li.Threshold values for free space and SWAPPER.DAT sizes
:li.Notification type (message box or alarm) when threshold value reached
:eul.
:p.After changing any settings, close the window. Select None to disable
automated refresh.

:h1 res=9711.Refresh Interval Help
:i2 refid=Autoref.Refresh Interval
:p.Click on the spin buttons for minutes to set the refresh
interval time. If you do not wish automated refresh, click on the None
push button.
:note.For Automated refreshes, you will not be prompted unlike the Refresh Now
option.

:h1 res=9712.Free Space Threshold Help
:i2 refid=Autoref.Free Space Threshold
:p.Click on the spin button to select the threshold percentage (of the
total disk space) for the disk free space. If the free space falls below
this percentage, you will be warned based on the notification type you
select.

:h1 res=9713.SWAPPER.DAT Threshold Help
:i2 refid=Autoref.SWAPPER.DAT Threshold
:p.Click on the spin button to select the threshold percentage (of the
total disk space) for the SWAPPER.DAT file. If the size of the SWAPPER.DAT
goes above
this percentage, you will be warned based on the notification type you
select.
:note.The SWAPPER.DAT threshold will only be checked if the current drive
contains the SWAPPER.DAT file.

:h1 res=9714.Free Space Warning Help
:i2 refid=Autoref.Free Space Warning
:p.You may select none, one or more of notification types
when the free space falls below the selected threshold limit. The following
warnings are available:
:ol.
:li.Alarm or beep
:li.Message box
:li.User exit command
:p.For example, :hp2.NET.EXE SEND user:ehp2. will send a message to the user
in the network. The message is the same message that was displayed in the
warning message box.
:eol.

:h1 res=9715.SWAPPER.DAT Warning Help
:i2 refid=Autoref.SWAPPER.DAT Warning
:p.You may select none, one or more of notification types
when the size of the SWAPPER.DAT increases
above the selected threshold limit. The following
warnings are available:
:ol.
:li.Alarm or beep
:li.Message box
:li.User exit command
:eol.
:note.The SWAPPER.DAT threshold will only be checked if the current drive
contains the SWAPPER.DAT file.

:h1 res=9716.None Help
:i2 refid=Autoref.None
:p.Click on the None push button if you do not want automated refresh.

:h1 res=9140.Move Directory
:i2 refid=Dirlist.Move
:p.Move all the contents (files) of the selected into a new directory.
The selected directory is deleted if possible. If the target directory is
not an existing directory, the target directory is created.

:h1 res=9141.Delete Directory
:i2 refid=Dirlist.Delete
:p.Delete the selected directory. The select directory and all its
subdirectories will be deleted.

:h1 res=9142.Copy Directory
:i2 refid=Dirlist.Copy
:p.Copy the selected into a new directory. If the target directory is
not an existing directory, the target directory is created.

:h1 res=9143.Make Root
:i2 refid=Dirlist.Make Root
:p.Make the selected directory the "virtual" root directory. The pie chart
and Directory List window will reflect that the selected directory is the
root directory. To switch back to the real root directory, click on the
Disk Root menu choice.

:h1 res=9144.Disk Root
:i2 refid=Dirlist.Disk Root
:p.Make the actual root directory the root directory.

:h1 res=9610.Target Directory Dialog
:i2 refid=Dirlist.Disk Target Directory
:p.Type the full directory name of the target directory. The format is:
:xmp.
  d&amp.\path
:exmp.

:h1 res=9135.Fonts Dialog Help
:i1 id=Fonts.Fonts Dialog
:p.You may select a new font name, type and size.
Only monospaced fonts can be selected, emphasis is ignored.

:h1 res=9810.Print Dialog Help
:i1 id=Print.Print Dialog
:p.Select one of the available printer queues. Then you may optionally select
to chance the job properties by clicking on the Job Properties push button.
:p.By default, the directory list will be printed only. You may print
the graph and legend by clicking on the Print Graph check box.
:p.When you are ready to print (make sure your printer is turned on!), click
on the Print push button to start printing.
:p.Note that during the printing process, you will not be able to change
drives or refresh.

:h1 res=9850.Stop Print Help
:i1 id=StopPrt.Stop Print
:p.Click on Yes if you really want to stop printing the set of bingo cards.
Otherwise, click No to continue.

:h1 res=9180.Help for Help
:i1 id=Help.Help Menu
:p.Use this choice to get help for the application. You may
request an index of help choices, general help for the application,
help for the supported function keys, or a brief description about the
application.

:h1 res=9182.Help for Help Index
:i2 refid=Help.Help Index
:p.Use this choice to get an index of available help topics.

:h1 res=9183.Help for General Help
:i2 refid=Help.General Help
:p.Use this choice to get general help for the whole application.

:h1 res=9184.Help for Using Help
:i2 refid=Help.Using Help
:p.Use this choice to get help in using the help subsystem itself.

:h1 res=9185.Help for Keys Help
:i2 refid=Help.Keys Help
:p.Use this choice to get help for the supported function keys.

:h1 res=9186.Help for Product Information
:i2 refid=Help.Product Information
:p.Use this choice to get a brief description of this program.

:h1 res=9187.Keys Help
:i1 id=keys.Keys
:p.The following keystrokes are valid in this application:
:table cols='8 20' rules=both frame=box.
:row.
:c.Key
:c.Description
:row.
:c.Ctrl+A
:c.Toggle all drives information
:row.
:c.Ctrl+C
:c.Toggle directory list
:row.
:c.Ctrl+L
:c.Toggle legend
:row.
:c.Ctrl+R
:c.Refresh
:row.
:c.Ctrl+S
:c.Settings
:row.
:c.F1
:c.Help
:etable.
:euserdoc.
